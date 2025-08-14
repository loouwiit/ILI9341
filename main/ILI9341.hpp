#pragma once

#include "gpio.hpp"
#include "spi.hpp"
#include "vector.hpp"

class ILI9341
{
public:
	constexpr static Vector2us ScreenSize = { 320, 240 };
	constexpr static size_t ScreenTotolSize = ScreenSize.y * ScreenSize.x;
	using Color = uint16_t;
	using Fream_t = Color[ScreenSize.y][ScreenSize.x];

	ILI9341() = default;
	ILI9341(ILI9341&) = delete;
	ILI9341& operator=(ILI9341&) = delete;

	ILI9341(ILI9341&& move);
	ILI9341& operator=(ILI9341&& move);

	ILI9341(SPI& host, GPIO dataCommandSelect, GPIO reset, GPIO CS, Fream_t* buffer, int speed = SPI_MASTER_FREQ_40M) :
		spi{ host, CS, 16, speed },
		freamBuffer{ buffer },
		dataCommandSelect{ dataCommandSelect, GPIO::Mode::GPIO_MODE_OUTPUT },
		resetGpio{ reset, GPIO::Mode::GPIO_MODE_OUTPUT }
	{}

	void reset();
	void init(Color color = 0x0000);

	void setAddressWindow(Vector2us start, Vector2us end);
	void drawPixel(Vector2us position, Color color);

	void drawLine(Vector2us start, Vector2us end, Color color = 0xFFFF);
	void drawRectangle(Vector2us start, Vector2us end, Color color = 0xFFFF);

	int drawText(Vector2us position, char text, Color textColor = 0xFFFF, Color backgroundColor = 0x0000);
	int drawText(Vector2us position, const char* text, Color textColor = 0xFFFF, Color backgroundColor = 0x0000);
	int drawNumber(Vector2us position, int number, unsigned base = 10, Color textColor = 0xFFFF, Color backgroundColor = 0x0000);
	int drawNumber(Vector2us position, unsigned number, unsigned base = 10, Color textColor = 0xFFFF, Color backgroundColor = 0x0000);

	void clear(Color color = 0x0000);
	void display();
	void waitForDisplay();

	void test();

protected:
	SPIDevice spi{};

	bool command(unsigned char command);
	bool command(unsigned char command, SPIDevice::SmallData_t data, int size);
	bool data(SPIDevice::SmallData_t data, int size = 1);
	bool data(char* data, size_t size);

	void drawModeStart();
	void drawModeContinue();

	static IRAM_ATTR void commandModeCallback(void* param);
	static IRAM_ATTR void dataModeCallback(void* param);

	Fream_t* freamBuffer = nullptr;

	GPIO dataCommandSelect{};
	GPIO resetGpio{};
};
