#pragma once

#include <type_traits>

#include "gpio.hpp"
#include "spi.hpp"
#include "vector.hpp"

#include "color.hpp"

template <class Color>
class ILI9341
{
public:
	static_assert(std::is_same<Color, Color565>::value || std::is_same<Color, Color666>::value);

	constexpr static Vector2us ScreenSize = { 320, 240 };
	constexpr static size_t ScreenTotolSize = ScreenSize.y * ScreenSize.x;

	using Frame_t = Color[ScreenSize.y][ScreenSize.x]; // DMA_ATTR please

	ILI9341() = default;
	ILI9341(ILI9341&) = delete;
	ILI9341& operator=(ILI9341&) = delete;

	ILI9341(ILI9341&& move);
	ILI9341& operator=(ILI9341&& move);

	ILI9341(SPI& host, GPIO dataCommandSelect, GPIO reset, GPIO CS, Frame_t* buffer, int speed = SPI_MASTER_FREQ_40M) :
		spi{ host, CS, 16, speed },
		frameBuffer{ buffer },
		dataCommandSelect{ dataCommandSelect, GPIO::Mode::GPIO_MODE_OUTPUT },
		resetGpio{ reset, GPIO::Mode::GPIO_MODE_OUTPUT }
	{}

	void reset();
	void init(Color color = Color::Black);

	void setAddressWindow(Vector2us start, Vector2us end);
	void drawPixel(Vector2us position, Color color);

	void drawLine(Vector2us start, Vector2us end, Color color = Color::White);
	void drawRectangle(Vector2us start, Vector2us end, Color color = Color::White);

	int drawText(Vector2us position, char text, Color textColor = Color::White, Color backgroundColor = Color::Black);
	int drawText(Vector2us position, const char* text, Color textColor = Color::White, Color backgroundColor = Color::Black);
	int drawNumber(Vector2us position, int number, unsigned base = 10, Color textColor = Color::White, Color backgroundColor = Color::Black);
	int drawNumber(Vector2us position, unsigned number, unsigned base = 10, Color textColor = Color::White, Color backgroundColor = Color::Black);

	void clear(Color color = Color::Black);
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

	Frame_t* frameBuffer = nullptr;

	GPIO dataCommandSelect{};
	GPIO resetGpio{};
};

#include <ILI9341.inl>
