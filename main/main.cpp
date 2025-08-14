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

	ILI9341::Color color = 0x0000;
	Vector2us position = { 0, 0 };

	while (position.y < lcd.ScreenSize.y && color < 0x001F)
	{
		lcd.drawPixel(position, (((color << 8) & 0xFF00) | ((color >> 8) & 0x00FF)));
		color += 0x0001;
		position.x++;
		if (position.x >= lcd.ScreenSize.x)
		{
			position.x = 0;
			position.y++;
		}
	}

	color = 0x0000;
	position = { 0, 100 };
	while (position.y < lcd.ScreenSize.y && color < 0x07E0)
	{
		lcd.drawPixel(position, (((color << 8) & 0xFF00) | ((color >> 8) & 0x00FF)));
		color += 0x0020;
		position.x++;
		if (position.x >= lcd.ScreenSize.x)
		{
			position.x = 0;
			position.y++;
		}
	}

	color = 0x0000;
	position = { 0, 200 };
	while (position.y < lcd.ScreenSize.y && color < 0xF800)
	{
		lcd.drawPixel(position, (((color << 8) & 0xFF00) | ((color >> 8) & 0x00FF)));
		color += 0x0800;
		position.x++;
		if (position.x >= lcd.ScreenSize.x)
		{
			position.x = 0;
			position.y++;
		}
	}

	lcd.display();
}
