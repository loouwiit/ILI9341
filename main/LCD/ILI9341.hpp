#pragma once

#include <type_traits>

#include "gpio.hpp"
#include "spi.hpp"
#include "vector.hpp"

#include "drawable.hpp"
#include "color.hpp"
#include "frame.hpp"
#include "layar.hpp"
#include "line.hpp"
#include "pixel.hpp"
#include "rectangle.hpp"
#include "text.hpp"

#define ILI9341_IRAM_ENABLE true
#if ILI9341_IRAM_ENABLE
#define ILI9341_IRAM IRAM_ATTR
#else
#define ILI9341_IRAM
#endif

template <ColorTemplate Color_t>
class ILI9341
{
public:
	using Color = Color_t;

	constexpr static Vector2us ScreenSize = { 320, 240 };
	constexpr static size_t ScreenTotolSize = ScreenSize.y * ScreenSize.x;

	using Frame = ::Frame<Color, ScreenSize>;
	template <unsigned char maxSize>
	using Layar = ::Layar<Color, ScreenSize, maxSize>;
	using Line = ::Line<Color, ScreenSize>;
	using Pixel = ::Pixel<Color, ScreenSize>;
	using Rectangle = ::Rectangle<Color, ScreenSize>;
	using Character = ::Character<Color, ScreenSize>;
	using Text = ::Text<Color, ScreenSize>;

	template <class T>
	using Number = ::Number<Color, ScreenSize, T>;

	ILI9341() = default;
	ILI9341(ILI9341&) = delete;
	ILI9341& operator=(ILI9341&) = delete;

	ILI9341(ILI9341&& move);
	ILI9341& operator=(ILI9341&& move);

	ILI9341(SPI& host, GPIO dataCommandSelect, GPIO reset, GPIO CS, Frame* frame, int speed = SPI_MASTER_FREQ_40M) :
		spi{ host, CS, 16, speed },
		frame{ frame },
		dataCommandSelect{ dataCommandSelect, GPIO::Mode::GPIO_MODE_OUTPUT },
		resetGpio{ reset, GPIO::Mode::GPIO_MODE_OUTPUT }
	{}

	operator Frame& () { return frame; }
	Vector2us draw(auto&& element);
	bool isDrawing();

	void reset();
	void init(Color color = Color::Black);

	void clear(Color color = Color::Black);
	void display(SPIDevice::function_t callBack = SPIDevice::emptyCallback, void* param = nullptr);
	void waitForDisplay(SPIDevice::waitFunction_t waitFunction = SPIDevice::sleepWait);

	void test();

protected:
	SPIDevice spi{};

	bool command(unsigned char command);
	bool command(unsigned char command, SPIDevice::SmallData_t data, int size);
	bool data(SPIDevice::SmallData_t data, int size = 1);
	bool data(char* data, size_t size);

	void setAddressWindow(Vector2us start, Vector2us end);

	void drawModeStart();
	void drawModeContinue();

	Frame* frame = nullptr;

	GPIO dataCommandSelect{};
	GPIO resetGpio{};
};

#include <ILI9341.inl>
