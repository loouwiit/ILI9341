#include <iostream>
#include <esp_log.h>

#include "ILI9341.hpp"
#include "FT6X36.hpp"
#include "app.hpp"
#include "touch/touch.hpp"
#include "clock/clock.hpp"

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
		while (lcd.isDrawing() || !app->drawMutex.try_lock())
			vTaskDelay(1);

		app->draw();
		app->drawMutex.unlock();
		lcd.display();
	}

	vTaskDelete(nullptr);
}

void touchThread(void*)
{
	while (true)
	{
		while (!touch.isNeedUpdate() || !app->touchMutex.try_lock())
			vTaskDelay(1);

		touch.update();
		app->touchUpdate();
		app->touchMutex.unlock();
	}

	vTaskDelete(nullptr);
}

void changeApp(App* nextApp)
{
	nextApp->init();

	App* oldApp = app;
	while (!oldApp->drawMutex.try_lock())
		vTaskDelay(1);
	while (!oldApp->touchMutex.try_lock())
		vTaskDelay(1);

	lcd.clear();
	app = nextApp;
	oldApp->drawMutex.unlock();
	oldApp->touchMutex.unlock();

	oldApp->deinit();
	while (!oldApp->isDeleteAble())
		vTaskDelay(1);

	delete oldApp;
}

void app_main(void)
{
	spi = SPI{ SPI2_HOST, {GPIO_NUM_NC}, {GPIO_NUM_13}, {GPIO_NUM_14} };
	lcd = LCD{ spi, {GPIO_NUM_21}, {GPIO_NUM_47}, {GPIO_NUM_48}, &screenBuffer };

	lcd.init(LCD::Color::White);
	lcd.draw(LCD::Rectangle{ {1,1},{318,238},LCD::Color::Black });

	GPIO::enableGlobalInterrupt();

	iic = IIC{ {GPIO_NUM_11}, {GPIO_NUM_12} };
	touch = FT6X36{ iic, {GPIO_NUM_10}, {GPIO_NUM_9} };

	touch.init();

	if (!touch.detectSelf())
	{
		ESP_LOGE(TAG, "touch not connected");
		lcd.draw(LCD::Text{ {10,10}, "touch not connected" });
		lcd.display();
		return;
	}

	app = new AppTouch{ lcd, touch, changeApp };
	app->init();

	xTaskCreate(drawThread, "draw", 4096, nullptr, 2, nullptr);
	xTaskCreate(touchThread, "touch", 4096, nullptr, 2, nullptr);

	// for debug only
	while (true)
	{
		vTaskDelay(10000);
		changeApp(new AppClock{ lcd,touch,changeApp });
		vTaskDelay(10000);
		changeApp(new AppTouch{ lcd, touch, changeApp });
	}
}
