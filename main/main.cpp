#include <iostream>
#include <esp_log.h>

#include "ILI9341.hpp"

extern "C" void app_main(void);

constexpr char TAG[] = "LCD";

constexpr size_t receiveBufferSize = 8;
constexpr size_t receiveBufferCount = 8192;

SPI spi{};
ILI9341 lcd{};

void app_main(void)
{
	spi = SPI{ SPI2_HOST, {GPIO_NUM_NC}, {GPIO_NUM_13}, {GPIO_NUM_14} };
	lcd = ILI9341{ spi, {GPIO_NUM_21}, {GPIO_NUM_47}, {GPIO_NUM_48} };

	lcd.init();

	vTaskDelay(100);

	lcd.drawRectangle({ 10,10 }, { 20,200 }, 0x0000);
	lcd.drawRectangle({ 40,10 }, { 40,200 }, 0xFFFF);
	lcd.drawRectangle({ 60,10 }, { 61,100 }, 0xF800); // R
	lcd.drawRectangle({ 60,101 }, { 61,200 }, 0x07E0); // G
	lcd.drawRectangle({ 60,200 }, { 61,239 }, 0x001F); // B

	vTaskDelay(10);
	// lcd.drawText({ 10,10 }, '0');

	while (true)
		lcd.test();
}
