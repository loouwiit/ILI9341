#include <iostream>
#include <esp_log.h>

#include "ILI9341.hpp"
#include "FT6X36.hpp"
#include "app.hpp"
#include "desktop/desktop.hpp"

extern "C" void app_main(void);

constexpr char TAG[] = "LCD";

using LCD = ILI9341<Color565>;

DMA_ATTR LCD::Frame screenBuffer;

SPI spi{};
LCD lcd{};

IIC iic{};
FT6X36 touch{};

App* app;

void drawThread(void*)
{
	while (true)
	{
		while (lcd.isDrawing())
			vTaskDelay(1);

		while (!app->drawMutex.try_lock())
			vTaskDelay(1);

		app->draw();
		app->drawMutex.unlock();
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

		while (!app->touchMutex.try_lock())
			vTaskDelay(1);

		touch.update();
		app->touchUpdate();
		app->touchMutex.unlock();
	}

	vTaskDelete(nullptr);
}

void changeApp(App* nextApp)
{
	static std::mutex changeMutex;
	static bool changing = false;

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
			App* nextApp = (App*)param;
			if (nextApp == nullptr)
				nextApp = new AppDesktop{ lcd, touch, changeApp };

			nextApp->init();

			App* oldApp = app;
			oldApp->touchMutex.lock();
			oldApp->drawMutex.lock(); // 但我们不释放

			lcd.clear();
			app = nextApp;

			oldApp->deinit();
			while (!oldApp->isDeleteAble())
				vTaskDelay(1);

			delete oldApp;
			changeMutex.lock();
			changing = false;
			changeMutex.unlock();
			vTaskDelete(nullptr);
		}
	, "appChangeThread", 4096, nextApp, 4, nullptr);
}

void app_main(void)
{
	spi = SPI{ SPI2_HOST, {GPIO_NUM_NC}, {GPIO_NUM_13}, {GPIO_NUM_14} };
	lcd = LCD{ spi, {GPIO_NUM_21}, {GPIO_NUM_47}, {GPIO_NUM_48}, &screenBuffer };

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
			constexpr clock_t LongPressClock = 3000;
			static auto pressTime = clock();
			if ((bool)GPIO { GPIO_NUM_0 } == false)
			{
				pressTime = clock();
				return;
			}

			auto nowTime = clock();
			if (nowTime - pressTime < LongPressClock)
			{
				app->back();
			}
			else
			{
				xTaskCreate([](void*)
				{
					while (!app->drawMutex.try_lock())
						vTaskDelay(1);
					lcd.waitForDisplay();
					lcd.init();
					lcd.waitForDisplay();
					app->drawMutex.unlock();
					vTaskDelete(nullptr);
				}
				, "lcdReset", 4096, nullptr, 4, nullptr);
			}
		}
	};

	app = new AppDesktop{ lcd, touch, changeApp };
	app->init();

	xTaskCreate(drawThread, "draw", 4096, nullptr, 2, nullptr);
	xTaskCreate(touchThread, "touch", 4096, nullptr, 2, nullptr);
}
