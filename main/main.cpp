#include <iostream>
#include <esp_log.h>

#include "ILI9341.hpp"

extern "C" void app_main(void);

constexpr char TAG[] = "LCD";

using LCD = ILI9341<Color565>;

DMA_ATTR LCD::Frame_t screenBuffer;

SPI spi{};
LCD lcd{};

void app_main(void)
{
	spi = SPI{ SPI2_HOST, {GPIO_NUM_NC}, {GPIO_NUM_13}, {GPIO_NUM_14} };
	lcd = LCD{ spi, {GPIO_NUM_21}, {GPIO_NUM_47}, {GPIO_NUM_48}, &screenBuffer };

	lcd.init();
	lcd.waitForDisplay();

	lcd.drawText({ 0,0 }, std::is_same<LCD, ILI9341<Color666>>::value ? "RGB666" : "RGB565");
	lcd.drawLine({ 100,0 }, { 100,239 });

	for (unsigned char i = 0; i < 64; i++)
	{
		lcd.drawPixel({ i, 050 }, { i,0,0 });
		lcd.drawPixel({ i, 100 }, { 0,i,0 });
		lcd.drawPixel({ i, 150 }, { 0,0,i });
	}

	lcd.test();
	lcd.waitForDisplay();

	auto lastTime = clock();
	unsigned deltaTime = 0;

	// 666 @ 40M : 46ms
	// 565 @ 40M : 30ms
	// 666 @ 80M~: 23ms
	// 565 @ 80M~: 15ms

	while (true)
	{
		vTaskDelay(1);
		lastTime = clock();
		lcd.display();
		lcd.waitForDisplay();
		deltaTime = clock() - lastTime;
		lcd.drawNumber({ 0,16 }, deltaTime);
	}
}
