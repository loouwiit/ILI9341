#pragma once

#include <type_traits>

#include "gpio.hpp"
#include "spi.hpp"
#include "vector.hpp"

#include "color.hpp"
#include "frame.hpp"

template <ColorTemplate Color>
class ILI9341
{
public:
	constexpr static Vector2us ScreenSize = { 320, 240 };
	constexpr static size_t ScreenTotolSize = ScreenSize.y * ScreenSize.x;

	using Frame = FrameBuffer<Color, ScreenSize>;

	ILI9341() = default;
	ILI9341(ILI9341&) = delete;
	ILI9341& operator=(ILI9341&) = delete;

	ILI9341(ILI9341&& move);
	ILI9341& operator=(ILI9341&& move);

	ILI9341(SPI& host, GPIO dataCommandSelect, GPIO reset, GPIO CS, Frame* buffer, int speed = SPI_MASTER_FREQ_40M) :
		spi{ host, CS, 16, speed },
		frame{ buffer },
		dataCommandSelect{ dataCommandSelect, GPIO::Mode::GPIO_MODE_OUTPUT },
		resetGpio{ reset, GPIO::Mode::GPIO_MODE_OUTPUT }
	{}

	Vector2us draw(auto&& element);

	void reset();
	void init(Color color = Color::Black);

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

	void setAddressWindow(Vector2us start, Vector2us end);

	void drawModeStart();
	void drawModeContinue();

	Frame* frame = nullptr;

	GPIO dataCommandSelect{};
	GPIO resetGpio{};
};

#include <ILI9341.inl>
