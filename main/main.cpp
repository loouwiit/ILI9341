#include <iostream>
#include <esp_log.h>

#include "ILI9341.hpp"

extern "C" void app_main(void);

constexpr char TAG[] = "LCD";

DMA_ATTR ILI9341::Frame_t screenBuffer;

SPI spi{};
ILI9341 lcd{};

void app_main(void)
{
	spi = SPI{ SPI2_HOST, {GPIO_NUM_NC}, {GPIO_NUM_13}, {GPIO_NUM_14} };
	lcd = ILI9341{ spi, {GPIO_NUM_21}, {GPIO_NUM_47}, {GPIO_NUM_48}, &screenBuffer };

	lcd.init();
	vTaskDelay(100);
	lcd.waitForDisplay();

	while (true)
	{
		lcd.clear(0xFFFF);
		lcd.display();
		vTaskDelay(100);
		lcd.waitForDisplay();

		lcd.drawRectangle({ 0,0 }, { 99,240 }, 0x00F8); // LE
		lcd.drawRectangle({ 100,0 }, { 199,240 }, 0xE007); // LE
		lcd.drawRectangle({ 200,0 }, { 319,240 }, 0x1F00); // LE
		lcd.drawNumber({ 0,0 }, -3254);
		lcd.drawText({ 0,50 }, "hello\nILI9341");
		lcd.drawLine({ 319,0 }, { 0,239 }, 0x0000);

		lcd.test();

		lcd.clear(0x0000);
		lcd.display();
		vTaskDelay(100);
		lcd.waitForDisplay();
	}
}
