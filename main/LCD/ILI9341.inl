#include "ILI9341.hpp"

#include <algorithm>
#include <esp_task.h>
#include <esp_log.h>

template <ColorTemplate Color>
ILI9341<Color>::ILI9341(ILI9341&& move)
{
	using std::swap;
	swap(move.spi, spi);
	swap(move.frame, frame);
	swap(move.dataCommandSelect, dataCommandSelect);
	swap(move.resetGpio, resetGpio);
}

template <ColorTemplate Color>
ILI9341<Color>& ILI9341<Color>::operator=(ILI9341&& move)
{
	using std::swap;
	swap(move.spi, spi);
	swap(move.frame, frame);
	swap(move.dataCommandSelect, dataCommandSelect);
	swap(move.resetGpio, resetGpio);
	return *this;
}

template <ColorTemplate Color>
Vector2us ILI9341<Color>::draw(auto&& element)
{
	return element.drawTo(*frame);
}

template<ColorTemplate Color_t>
inline bool ILI9341<Color_t>::isDrawing()
{
	return spi.getTransmittingCount() > 0;
}

template <ColorTemplate Color>
void ILI9341<Color>::reset()
{
	resetGpio = false;
	vTaskDelay(1 / portTICK_PERIOD_MS);
	resetGpio = true;
	vTaskDelay(150 / portTICK_PERIOD_MS);
}

template <ColorTemplate Color>
void ILI9341<Color>::init(Color color)
{
	reset();
	command(0x11); // sleep out
	spi.waitForTransmition();
	vTaskDelay(10 / portTICK_PERIOD_MS);

	command(0xC0, { 0x0A }, 1); // power contorl 1
	command(0x36, { 0x20 | 0x08 }, 1); // Memory Access Control : XY反转，BGR模式

	if constexpr (std::is_same<Color, Color565>::value)
	{
		command(0x3A, { 0x55 }, 1); // 16 bit
	}

	command(0xC5, { 0x10,0x4C }, 2); // VCM1

	command(0x21); // inverse on

	command(0x29); // display on

	clear(color);
	spi.waitForTransmition();
	display();
}

template <ColorTemplate Color>
bool ILI9341<Color>::command(unsigned char command)
{
	void ILI9341_IRAM gpioClearCallback(void*);
	return spi.transmit({ (char)command }, 8, gpioClearCallback, &dataCommandSelect);
}

template <ColorTemplate Color>
bool ILI9341<Color>::command(unsigned char command, SPIDevice::SmallData_t data, int size)
{
	void ILI9341_IRAM gpioClearCallback(void*);
	void ILI9341_IRAM gpioSetCallback(void*);
	if (!spi.transmit({ (char)command }, 8, gpioClearCallback, &dataCommandSelect))
		return false;
	return spi.transmit(data, size * 8, gpioSetCallback, &dataCommandSelect);
}

template <ColorTemplate Color>
bool ILI9341<Color>::data(SPIDevice::SmallData_t data, int size)
{
	void ILI9341_IRAM gpioSetCallback(void*);
	return spi.transmit(data, size * 8, gpioSetCallback, &dataCommandSelect);
}

template <ColorTemplate Color>
bool ILI9341<Color>::data(char* data, size_t size)
{
	void ILI9341_IRAM gpioSetCallback(void*);
	return spi.transmit(data, size * 8, gpioSetCallback, &dataCommandSelect);
}

template <ColorTemplate Color>
void ILI9341<Color>::drawModeStart()
{
	command(0x2C);
}

template <ColorTemplate Color>
void ILI9341<Color>::drawModeContinue()
{
	command(0x3C);
}

template <ColorTemplate Color>
void ILI9341<Color>::setAddressWindow(Vector2us start, Vector2us end)
{
	command(0x2A, { (char)(start.x >> 8), (char)(start.x & 0xFF), (char)((end.x) >> 8), (char)((end.x) & 0xFF) }, 4);
	command(0x2B, { (char)(start.y >> 8), (char)(start.y & 0xFF), (char)((end.y) >> 8), (char)((end.y) & 0xFF) }, 4);
}

template <ColorTemplate Color>
void ILI9341<Color>::clear(Color color)
{
	std::fill(&(*frame)[0][0], &(*frame)[0][0] + ScreenTotolSize, color);
}

template <ColorTemplate Color>
void ILI9341<Color>::display(SPIDevice::function_t callBack, void* param)
{
	setAddressWindow({ 0,0 }, { ScreenSize.x - 1, ScreenSize.y - 1 });
	drawModeStart();

	constexpr size_t sendStep = ScreenTotolSize / (std::is_same<Color, Color565>::value ? 5 : 8);

	void ILI9341_IRAM gpioSetCallback(void*);
	spi.transmit(&(*frame)[0][0] + sendStep * 0, sendStep * sizeof(Color) * 8, gpioSetCallback, &dataCommandSelect);
	spi.transmit(&(*frame)[0][0] + sendStep * 1, sendStep * sizeof(Color) * 8);
	spi.transmit(&(*frame)[0][0] + sendStep * 2, sendStep * sizeof(Color) * 8);
	spi.transmit(&(*frame)[0][0] + sendStep * 3, sendStep * sizeof(Color) * 8);

	if constexpr (std::is_same<Color, Color565>::value)
	{
		spi.transmit(&(*frame)[0][0] + sendStep * 4, sendStep * sizeof(Color) * 8, SPIDevice::emptyCallback, nullptr, callBack, param);
	}
	else if constexpr (std::is_same<Color, Color666>::value)
	{
		spi.transmit(&(*frame)[0][0] + sendStep * 4, sendStep * sizeof(Color) * 8);
		spi.transmit(&(*frame)[0][0] + sendStep * 5, sendStep * sizeof(Color) * 8);
		spi.transmit(&(*frame)[0][0] + sendStep * 6, sendStep * sizeof(Color) * 8);
		spi.transmit(&(*frame)[0][0] + sendStep * 7, sendStep * sizeof(Color) * 8, SPIDevice::emptyCallback, nullptr, callBack, param);
	}
	else
	{
		static_assert(false, "仅支持Color565和Color666");
	}
}

template <ColorTemplate Color>
void ILI9341<Color>::waitForDisplay(SPIDevice::waitFunction_t waitFunction)
{
	spi.waitForTransmition(waitFunction);
}

#include "text.hpp"

template <ColorTemplate Color>
void ILI9341<Color>::test()
{
	draw(Text{ { 105,10 }, "QWERTYUIO\nPASDFGHJK\nLZXCVBNM", Color::White, Color::Blue });
	draw(Text{ { 185,10 }, "qwertyuio\npasdfghjk\nlzxcvbnm", Color::White, Color::Blue });

	draw(Text{ { 105,70 }, "123456789\n0!@#$%^&*\n()`~-=_+", Color::White, Color::Blue });
	draw(Text{ { 185,70 }, "[]{}|\\:;\n\"'<>,.?/\nLCD OK" , Color::White, Color::Blue });

	draw(Text{ { 105,150 }, "The quick brown\nfox jumps over\nthe lazy dog", Color::White, Color::Blue });

	display();
	vTaskDelay(3000 / portTICK_PERIOD_MS);

	draw(Text{ { 105,150 }, "\nfox\t\t\t\t\t\t\t    \n         dog", Color::White, Color::Blue });

	waitForDisplay();
	display();
	vTaskDelay(2000 / portTICK_PERIOD_MS);
}
