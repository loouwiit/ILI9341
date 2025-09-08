#include <iostream>
#include <esp_log.h>
#include <esp_event.h>
#include "wifi/nvs.hpp"

#include "LCD/ILI9341.hpp"
#include "LCD/FT6X36.hpp"
#include "app.hpp"
#include "desktop/desktop.hpp"

#include "storge/fat.hpp"
#include "storge/mem.hpp"
#include "storge/sd.hpp"

#define ChangeAppLog false

extern "C" void app_main(void);

constexpr char TAG[] = "LCD";

using LCD = ILI9341<Color565>;

DMA_ATTR LCD::Frame screenBuffer;

static SPI spi{};
static LCD lcd{};

EXT_RAM_BSS_ATTR static IIC iic{};
EXT_RAM_BSS_ATTR static FT6X36 touch{};

EXT_RAM_BSS_ATTR static App* app[10]{};
EXT_RAM_BSS_ATTR static unsigned char appIndex = 0;
EXT_RAM_BSS_ATTR static Mutex changeMutex{};
EXT_RAM_BSS_ATTR static bool changing = false;

void drawThread(void*)
{
	while (true)
	{
		while (lcd.isDrawing())
			vTaskDelay(1);

		while (!app[appIndex]->drawMutex.try_lock())
			vTaskDelay(1);

		app[appIndex]->draw();
		app[appIndex]->drawMutex.unlock();
		lcd.display();
	}

	vTaskDelete(nullptr);
}

void touchThread(void*)
{
	constexpr unsigned char timeOutCount = 100;
	unsigned char count = timeOutCount;
	while (true)
	{
		while (!touch.isNeedUpdate())
		{
			if (count < timeOutCount)
			{
				count++; // 超时判断
				if (count == timeOutCount) break;
			}
			vTaskDelay(1);
		}

		// 有数据 启用超时判断
		if (touch.isNeedUpdate()) count = 0;

		while (!app[appIndex]->touchMutex.try_lock())
			vTaskDelay(1);

		touch.update();
		app[appIndex]->touchUpdate();
		app[appIndex]->touchMutex.unlock();
	}

	vTaskDelete(nullptr);
}

void changeAppCallback(App* nextApp)
{
#if ChangeAppLog
	constexpr static char TAG[] = "changeAppCallback";
#endif

	while (true)
	{
		changeMutex.lock();
		if (changing)
		{
			changeMutex.unlock();
#if ChangeAppLog
			ESP_LOGI(TAG, "already in change");
#endif
			vTaskDelay(1);
			continue;
		}
		break;
	}
	changing = true;
	changeMutex.unlock();

	xTaskCreate([](void* param)
		{
			App* oldApp = app[appIndex];
			oldApp->touchMutex.lock(); // POSIX标准锁的析构必须释放状态
			oldApp->drawMutex.lock(); // 记得释放
			lcd.clear();

			App* nextApp = (App*)param;
			if (nextApp == nullptr)
			{
				// exit back
				if (appIndex == 0)
				{
					// back to desktop
					void newAppCallback(App * nextApp);
					nextApp = new AppDesktop{ lcd, touch, changeAppCallback, newAppCallback };
					nextApp->init();
					app[appIndex] = nextApp;
#if ChangeAppLog
					ESP_LOGI(TAG, "no back app, new desktop @ %p", app[appIndex]);
#endif
				}
				else
				{
					// back to last one
					appIndex--;
					app[appIndex]->focusIn();
#if ChangeAppLog
					ESP_LOGI(TAG, "back to last app @ %p", app[appIndex]);
#endif
				}
			}
			else
			{
				// change to new app
				nextApp->init();
				app[appIndex] = nextApp;
#if ChangeAppLog
				ESP_LOGI(TAG, "change to app @ %p", app[appIndex]);
#endif
			}

			// 删除旧app
			oldApp->deinit();
			while (!oldApp->isDeleteAble())
				vTaskDelay(1);

			vTaskDelay(1);
			oldApp->drawMutex.unlock();
			oldApp->touchMutex.unlock();

#if ChangeAppLog
			ESP_LOGI(TAG, "delete old app @ %p", oldApp);
#endif
			delete oldApp;
			changeMutex.lock();
			changing = false;
			changeMutex.unlock();
			vTaskDelete(nullptr);
		}
	, "changeAppThread", 4096, nextApp, 4, nullptr);
}

void newAppCallback(App* nextApp)
{
#if ChangeAppLog
	constexpr static char TAG[] = "newAppCallback";
#endif

	while (true)
	{
		changeMutex.lock();
		if (changing)
		{
			changeMutex.unlock();
			vTaskDelay(1);
			continue;
		}
		break;
	}
	changing = true;
	changeMutex.unlock();

	xTaskCreate([](void* param)
		{
			App* oldApp = app[appIndex];
			oldApp->touchMutex.lock(); // 旧app要锁住
			oldApp->drawMutex.lock(); // 确保渲染和触摸不会乱
			lcd.clear();

			App* nextApp = (App*)param;
			if (nextApp == nullptr)
			{
				// new nullptr ???
#if ChangeAppLog
				ESP_LOGE(TAG, "new app with nullptr");
#endif
				oldApp->drawMutex.unlock();
				oldApp->touchMutex.unlock();
			}
			else
			{
				// move to new app
				nextApp->init();
				app[appIndex + 1] = nextApp;
				appIndex++;
#if ChangeAppLog
				ESP_LOGI(TAG, "new app @ %p", app[appIndex]);
#endif
			}

			changeMutex.lock();
			changing = false;
			changeMutex.unlock();

			oldApp->drawMutex.unlock();
			oldApp->touchMutex.unlock();

			vTaskDelete(nullptr);
		}
	, "newAppThread", 4096, nextApp, 4, nullptr);
}

void app_main(void)
{
	spi = SPI{ SPI2_HOST, {GPIO_NUM_45}, {GPIO_NUM_13}, {GPIO_NUM_14} };
	lcd = LCD{ spi, {GPIO_NUM_21}, {GPIO_NUM_47}, {GPIO_NUM_48}, &screenBuffer };

	ESP_ERROR_CHECK(esp_event_loop_create_default());
	nvsInit();
	setenv("TZ", "CST-8", 1);

	mountFlash();
	mountMem();
	xTaskCreate([](void*) { mountSd(spi, { GPIO_NUM_3 }); vTaskDelete(nullptr); }, "mount sd", 4096, nullptr, 1, nullptr);

	vTaskDelay(1);
	lcd.init();

	GPIO::enableGlobalInterrupt();

	iic = IIC{ {GPIO_NUM_11}, {GPIO_NUM_12} };
	touch = FT6X36{ iic, {GPIO_NUM_10}, {GPIO_NUM_9} };

	touch.init();

	if (!touch.detectSelf())
	{
		ESP_LOGE(TAG, "touch not connected");
		lcd.draw(LCD::Text{ {10,10}, "touch not connected" });
		lcd.waitForDisplay();
		lcd.display();
		return;
	}

	GPIO{ GPIO_NUM_0, GPIO::Mode::GPIO_MODE_INPUT, GPIO::Pull::GPIO_PULLUP_ONLY, GPIO::Interrupt::GPIO_INTR_ANYEDGE, [](void*)
		{
			constexpr clock_t LongPressClock = 1000;
			static auto pressTime = clock();
			if ((bool)GPIO { GPIO_NUM_0 } == false)
			{
				pressTime = clock();
				return;
			}

			auto nowTime = clock();
			if (nowTime - pressTime < LongPressClock)
			{
				app[appIndex]->back();
			}
			else
			{
				xTaskCreate([](void*)
				{
					while (!app[appIndex]->drawMutex.try_lock())
						vTaskDelay(1);
					lcd.waitForDisplay();
					ESP_LOGI(TAG, "re-init");
					touch.restart();
					lcd.init();
					lcd.waitForDisplay();
					app[appIndex]->drawMutex.unlock();
					vTaskDelete(nullptr);
				}
				, "lcdReset", 4096, nullptr, 4, nullptr);
			}
		}
	};

	app[0] = new AppDesktop{ lcd, touch, changeAppCallback, newAppCallback };
	app[0]->init();

	xTaskCreate(drawThread, "draw", 4096, nullptr, 2, nullptr);
	xTaskCreate(touchThread, "touch", 4096, nullptr, 2, nullptr);
}
