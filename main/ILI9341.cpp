#include "ILI9341.hpp"

#include <algorithm>
#include <esp_task.h>

#include "font.hpp"

void IRAM_ATTR ILI9341::commandModeCallback(void* param)
{
	auto& dc = *(GPIO*)param;
	dc = false;
}

void IRAM_ATTR ILI9341::dataModeCallback(void* param)
{
	auto& dc = *(GPIO*)param;
	dc = true;
}

static bool sleepOutWhillWait()
{
	vTaskDelay(1);
	return true;
}

ILI9341::ILI9341(ILI9341&& move)
{
	using std::swap;
	swap(move.spi, spi);
	swap(move.dataCommandSelect, dataCommandSelect);
	swap(move.resetGpio, resetGpio);
}

ILI9341& ILI9341::operator=(ILI9341&& move)
{
	using std::swap;
	swap(move.spi, spi);
	swap(move.dataCommandSelect, dataCommandSelect);
	swap(move.resetGpio, resetGpio);
	return *this;
}

void ILI9341::reset()
{
	resetGpio = false;
	vTaskDelay(1 / portTICK_PERIOD_MS);
	resetGpio = true;
	vTaskDelay(150 / portTICK_PERIOD_MS);
}

void ILI9341::init(Color color)
{
	reset();
	command(0x11); // sleep out
	spi.waitForTransmition(sleepOutWhillWait);
	vTaskDelay(10 / portTICK_PERIOD_MS);

	command(0xC0, { 0x0A }, 1); // power contorl 1
	command(0x36, { 0x20 | 0x08 }, 1); // Memory Access Control : XY反转，BGR模式
	command(0x3A, { 0x55 }, 1); // 16 bit
	command(0x21); // inverse on

	command(0x29); // display on

	// clear(color);
	display();
}

bool ILI9341::command(unsigned char command)
{
	return spi.transmit({ (char)command }, 8, commandModeCallback, &dataCommandSelect);
}

bool ILI9341::command(unsigned char command, SPIDevice::SmallData_t data, int size)
{
	if (!spi.transmit({ (char)command }, 8, commandModeCallback, &dataCommandSelect))
		return false;
	return spi.transmit(data, size * 8, dataModeCallback, &dataCommandSelect);
}

bool ILI9341::data(SPIDevice::SmallData_t data, int size)
{
	return spi.transmit(data, size * 8, dataModeCallback, &dataCommandSelect);
}

bool ILI9341::data(char* data, size_t size)
{
	return spi.transmit(data, size * 8, dataModeCallback, &dataCommandSelect);
}

void ILI9341::drawModeStart()
{
	command(0x2C);
}

void ILI9341::drawModeContinue()
{
	command(0x3C);
}

void ILI9341::setAddressWindow(Vector2us start, Vector2us end)
{
	command(0x2A, { (char)(start.x >> 8), (char)(start.x & 0xFF), (char)((end.x) >> 8), (char)((end.x) & 0xFF) }, 4);
	command(0x2B, { (char)(start.y >> 8), (char)(start.y & 0xFF), (char)((end.y) >> 8), (char)((end.y) & 0xFF) }, 4);
}

void ILI9341::drawPixel(Vector2us position, Color color)
{
	setAddressWindow(position, position);
	drawModeStart();
	data({ (char)(color >> 8), (char)(color & 0xFF) }, 2);
}

void ILI9341::drawLine(Vector2us start, Vector2us end, Color color)
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

void ILI9341::drawRectangle(Vector2us start, Vector2us end, Color color)
{
	unsigned count = (end.x - start.x + 1) * (end.y - start.y + 1);
	count = (count + 1) / 2;

	SPIDevice::SmallData_t colorBuffer = { (char)(color >> 8), (char)(color & 0xFF),(char)(color >> 8), (char)(color & 0xFF) };

	setAddressWindow(start, end);
	drawModeStart();

	for (unsigned i = 0; i < count; i++)
		data(colorBuffer, 4);
}

// void LCD::drawTriangle(Vector2us position[3], Color color)
// {
// 	if (position[0].y > position[1].y) position[0].swap(position[1]);
// 	if (position[0].y > position[2].y) position[0].swap(position[2]);
// 	if (position[1].y > position[2].y) position[1].swap(position[2]);
// 	// 0 <= 1 <= 2
// }

int ILI9341::drawText(Vector2us position, char text, Color textColor, Color backgroundColor)
{
	if (text < 0x20) return 0;
	text -= 0x20;

	const unsigned char* font = fonts[(unsigned char)text];

	SPIDevice::SmallData_t textColorBuffer = { (char)(textColor >> 8), (char)(textColor & 0xFF) };
	SPIDevice::SmallData_t backgroundColorBuffer = { (char)(backgroundColor >> 8), (char)(backgroundColor & 0xFF) };

	setAddressWindow(position, position + Vector2us{ 7,15 });
	drawModeStart();

	for (unsigned char i = 0;i < 16;i++)
	{
		const unsigned char& mod = font[i];
		for (unsigned char j = 0; j < 8; j++)
		{
			bool draw = mod & ((1 << 7) >> j);
			if (draw)
			{
				data(textColorBuffer, 2);
			}
			else
			{
				data(backgroundColorBuffer, 2);
			}
		}
	}
	return 1;
}

int ILI9341::drawText(Vector2us position, const char* text, Color textColor, Color BackgroundColor)
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

int ILI9341::drawNumber(Vector2us position, int number, unsigned base, Color textColor, Color backgroundColor)
{
	if (number < 0)
	{
		drawText(position, '-', textColor, backgroundColor);
		return 1 + drawNumber(position + Vector2us{ 8, 0 }, (unsigned)-number, base, textColor, backgroundColor);
	}
	else return drawNumber(position, (unsigned)number, base, textColor, backgroundColor);
}

int ILI9341::drawNumber(Vector2us position, unsigned number, unsigned base, Color textColor, Color backgroundColor)
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

void ILI9341::clear(Color color)
{
	drawRectangle({ 0,0 }, { 320,240 }, color);
}

void ILI9341::display()
{
	spi.waitForTransmition();
}

void ILI9341::test()
{
	drawText({ 5,10 }, "QWERTYUIO\nPASDFGHJK\nLZXCVBNM", 0xFFFF, 0x00FF);
	drawText({ 85,10 }, "qwertyuio\npasdfghjk\nlzxcvbnm", 0xFFFF, 0x00FF);

	display();
	vTaskDelay(2000 / portTICK_PERIOD_MS);

	drawText({ 5,10 }, "         \n         \n         ");
	drawText({ 85,10 }, "         \n         \n         ");
	drawText({ 5,10 }, "123456789\n0!@#$%^&*\n()`~-=_+");
	drawText({ 85,10 }, "[]{}|\\:;\n\"'<>,.?/\nLCD OK");

	display();
	vTaskDelay(2000 / portTICK_PERIOD_MS);

	drawText({ 5,10 }, "         \n         \n         ");
	drawText({ 85,10 }, "         \n         \n         ");
	drawText({ 5,10 }, "The quick brown\nfox jumps over\nthe lazy dog");

	display();
	vTaskDelay(1500 / portTICK_PERIOD_MS);

	drawText({ 5,10 }, "\nfox\t\t\t\t\t\t\t    \n         dog", 0x0000, 0x00FF);

	display();
	vTaskDelay(500 / portTICK_PERIOD_MS);

	drawText({ 5,10 }, "               \n              \n            ");
	display();
	vTaskDelay(500 / portTICK_PERIOD_MS);
}
