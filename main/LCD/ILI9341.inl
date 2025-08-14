#include "ILI9341.hpp"

#include <algorithm>
#include <esp_task.h>
#include <esp_log.h>

#include "font.hpp"

template <class Color>
ILI9341<Color>::ILI9341(ILI9341&& move)
{
	using std::swap;
	swap(move.spi, spi);
	swap(move.frameBuffer, frameBuffer);
	swap(move.dataCommandSelect, dataCommandSelect);
	swap(move.resetGpio, resetGpio);
}

template <class Color>
ILI9341<Color>& ILI9341<Color>::operator=(ILI9341&& move)
{
	using std::swap;
	swap(move.spi, spi);
	swap(move.frameBuffer, frameBuffer);
	swap(move.dataCommandSelect, dataCommandSelect);
	swap(move.resetGpio, resetGpio);
	return *this;
}

template <class Color>
void ILI9341<Color>::reset()
{
	resetGpio = false;
	vTaskDelay(1 / portTICK_PERIOD_MS);
	resetGpio = true;
	vTaskDelay(150 / portTICK_PERIOD_MS);
}

template <class Color>
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

	command(0x21); // inverse on

	command(0x29); // display on

	clear(color);
	spi.waitForTransmition();
	display();
}

template <class Color>
bool ILI9341<Color>::command(unsigned char command)
{
	void IRAM_ATTR gpioClearCallback(void*);
	return spi.transmit({ (char)command }, 8, gpioClearCallback, &dataCommandSelect);
}

template <class Color>
bool ILI9341<Color>::command(unsigned char command, SPIDevice::SmallData_t data, int size)
{
	void IRAM_ATTR gpioClearCallback(void*);
	void IRAM_ATTR gpioSetCallback(void*);
	if (!spi.transmit({ (char)command }, 8, gpioClearCallback, &dataCommandSelect))
		return false;
	return spi.transmit(data, size * 8, gpioSetCallback, &dataCommandSelect);
}

template <class Color>
bool ILI9341<Color>::data(SPIDevice::SmallData_t data, int size)
{
	void IRAM_ATTR gpioSetCallback(void*);
	return spi.transmit(data, size * 8, gpioSetCallback, &dataCommandSelect);
}

template <class Color>
bool ILI9341<Color>::data(char* data, size_t size)
{
	void IRAM_ATTR gpioSetCallback(void*);
	return spi.transmit(data, size * 8, gpioSetCallback, &dataCommandSelect);
}

template <class Color>
void ILI9341<Color>::drawModeStart()
{
	command(0x2C);
}

template <class Color>
void ILI9341<Color>::drawModeContinue()
{
	command(0x3C);
}

template <class Color>
void ILI9341<Color>::setAddressWindow(Vector2us start, Vector2us end)
{
	command(0x2A, { (char)(start.x >> 8), (char)(start.x & 0xFF), (char)((end.x) >> 8), (char)((end.x) & 0xFF) }, 4);
	command(0x2B, { (char)(start.y >> 8), (char)(start.y & 0xFF), (char)((end.y) >> 8), (char)((end.y) & 0xFF) }, 4);
}

template <class Color>
void ILI9341<Color>::drawPixel(Vector2us position, Color color)
{
	(*frameBuffer)[position.y][position.x] = color;
}

template <class Color>
void ILI9341<Color>::drawLine(Vector2us start, Vector2us end, Color color)
{
	using Vector2s = Vector2<signed short>;
	Vector2s delta = (Vector2s)end - (Vector2s)start;
	Vector2us absDelta = { (unsigned char)abs(delta.x), (unsigned char)abs(delta.y) };
	if (absDelta.x > absDelta.y)
	{
		if (delta.x < 0)
		{
			start.swap(end);
			delta = -delta;
		}

		float k = (float)delta.y / (float)delta.x;

		Vector2f position = start;
		while ((int)position.x <= end.x)
		{
			drawPixel((Vector2us)position, color);
			position.x++;
			position.y += k;
		}
	}
	else
	{
		if (delta.y < 0)
		{
			start.swap(end);
			delta = -delta;
		}

		float l = (float)delta.x / (float)delta.y;

		Vector2f position = start;
		while ((int)position.y <= end.y)
		{
			drawPixel((Vector2us)position, color);
			position.y++;
			position.x += l;
		}
	}
}

template <class Color>
void ILI9341<Color>::drawRectangle(Vector2us start, Vector2us end, Color color)
{
	while (start.y <= end.y)
	{
		std::fill(&(*frameBuffer)[start.y][start.x], &(*frameBuffer)[start.y][end.x] + 1, color);
		start.y++;
	}

}

template <class Color>
int ILI9341<Color>::drawText(Vector2us position, char text, Color textColor, Color backgroundColor)
{
	if (text < 0x20) return 0;
	text -= 0x20;

	const unsigned char* font = fonts[(unsigned char)text];

	for (unsigned char i = 0;i < 16;i++)
	{
		const unsigned char& mod = font[i];
		for (unsigned char j = 0; j < 8; j++)
		{
			bool draw = mod & ((1 << 7) >> j);
			if (draw)
				(*frameBuffer)[position.y][position.x] = textColor;
			else
				(*frameBuffer)[position.y][position.x] = backgroundColor;

			position.x++;
		}
		position.x -= 8;
		position.y++;
	}
	return 1;
}

template <class Color>
int ILI9341<Color>::drawText(Vector2us position, const char* text, Color textColor, Color BackgroundColor)
{
	int drawCount = 0;

	Vector2us lineBegin = position;
	for (; *text != '\0'; text++)
	{
		switch (*text)
		{
		case '\n':
			position = lineBegin + Vector2us{ 0, 16 };
			lineBegin = position;
			continue;
		case '\r':
			position = lineBegin;
			continue;
		case '\b':
			position.x -= 8;
			continue;
		case '\t':
			position.x += 8;
			continue;
		}
		drawCount += drawText(position, *text, textColor, BackgroundColor);

		position.x += 8;
	}

	return drawCount;
}

template <class Color>
int ILI9341<Color>::drawNumber(Vector2us position, int number, unsigned base, Color textColor, Color backgroundColor)
{
	if (number < 0)
	{
		drawText(position, '-', textColor, backgroundColor);
		return 1 + drawNumber(position + Vector2us{ 8, 0 }, (unsigned)-number, base, textColor, backgroundColor);
	}
	else return drawNumber(position, (unsigned)number, base, textColor, backgroundColor);
}

template <class Color>
int ILI9341<Color>::drawNumber(Vector2us position, unsigned number, unsigned base, Color textColor, Color backgroundColor)
{
	if (number == 0)
	{
		drawText(position, '0', textColor, backgroundColor);
		return 1;
	}
	int drawCount = 0;
	auto draw = [this, &base, &position, &textColor, &backgroundColor, &drawCount](unsigned number, auto draw)->void
		{
			constexpr char HexNumber[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
			if (number >= base)
			{
				draw(number / base, draw);
				position.x += 8;
			}
			this->drawText(position, HexNumber[number % base], textColor, backgroundColor);
			drawCount++;
		};
	draw(number, draw);
	return drawCount;
}

template <class Color>
void ILI9341<Color>::clear(Color color)
{
	drawRectangle({ 0,0 }, { 319,239 }, color);
}

template <class Color>
void ILI9341<Color>::display()
{
	setAddressWindow({ 0,0 }, { 319,239 });
	drawModeStart();

	constexpr size_t sendStep = ScreenTotolSize / (std::is_same<Color, Color565>::value ? 5 : 8);

	void IRAM_ATTR gpioSetCallback(void*);
	spi.transmit(&(*frameBuffer)[0][0] + sendStep * 0, sendStep * sizeof(Color) * 8, gpioSetCallback, &dataCommandSelect);
	spi.transmit(&(*frameBuffer)[0][0] + sendStep * 1, sendStep * sizeof(Color) * 8, gpioSetCallback, &dataCommandSelect);
	spi.transmit(&(*frameBuffer)[0][0] + sendStep * 2, sendStep * sizeof(Color) * 8, gpioSetCallback, &dataCommandSelect);
	spi.transmit(&(*frameBuffer)[0][0] + sendStep * 3, sendStep * sizeof(Color) * 8, gpioSetCallback, &dataCommandSelect);
	spi.transmit(&(*frameBuffer)[0][0] + sendStep * 4, sendStep * sizeof(Color) * 8, gpioSetCallback, &dataCommandSelect);

	if constexpr (std::is_same<Color, Color666>::value)
	{
		spi.transmit(&(*frameBuffer)[0][0] + sendStep * 5, sendStep * sizeof(Color) * 8, gpioSetCallback, &dataCommandSelect);
		spi.transmit(&(*frameBuffer)[0][0] + sendStep * 6, sendStep * sizeof(Color) * 8, gpioSetCallback, &dataCommandSelect);
		spi.transmit(&(*frameBuffer)[0][0] + sendStep * 7, sendStep * sizeof(Color) * 8, gpioSetCallback, &dataCommandSelect);
	}
}

template <class Color>
void ILI9341<Color>::waitForDisplay()
{
	spi.waitForTransmition();
}

template <class Color>
void ILI9341<Color>::test()
{
	drawText({ 105,10 }, "QWERTYUIO\nPASDFGHJK\nLZXCVBNM", Color::White, Color::Blue); // LE
	drawText({ 185,10 }, "qwertyuio\npasdfghjk\nlzxcvbnm", Color::White, Color::Blue); // LE

	drawText({ 105,70 }, "123456789\n0!@#$%^&*\n()`~-=_+");
	drawText({ 185,70 }, "[]{}|\\:;\n\"'<>,.?/\nLCD OK");

	drawText({ 105,150 }, "The quick brown\nfox jumps over\nthe lazy dog");

	display();
	vTaskDelay(3000 / portTICK_PERIOD_MS);

	drawText({ 105,150 }, "\nfox\t\t\t\t\t\t\t    \n         dog", Color::White, Color::Blue); // LE

	display();
	vTaskDelay(2000 / portTICK_PERIOD_MS);
}
