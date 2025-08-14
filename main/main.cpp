#include <iostream>
#include <esp_log.h>

#include "ILI9341.hpp"

extern "C" void app_main(void);

constexpr char TAG[] = "LCD";

using LCD = ILI9341<Color666>;

DMA_ATTR LCD::Frame_t screenBuffer;

SPI spi{};
LCD lcd{};

void app_main(void)
{
	spi = SPI{ SPI2_HOST, {GPIO_NUM_NC}, {GPIO_NUM_13}, {GPIO_NUM_14} };
	lcd = LCD{ spi, {GPIO_NUM_21}, {GPIO_NUM_47}, {GPIO_NUM_48}, &screenBuffer };

	lcd.init();
	lcd.waitForDisplay();

	lcd.drawText({ 0,0 }, std::is_same<LCD, ILI9341<Color666>>::value ? "666" : "565");

	for (unsigned char i = 0; i < 64; i++)
	{
		lcd.drawPixel({ i, 050 }, { i,0,0 });
		lcd.drawPixel({ i, 100 }, { 0,i,0 });
		lcd.drawPixel({ i, 150 }, { 0,0,i });
	}

	lcd.test();
}
