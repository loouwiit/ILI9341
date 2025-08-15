#include <iostream>
#include <esp_log.h>

#include "ILI9341.hpp"

#include "pixel.hpp"
#include "line.hpp"
#include "text.hpp"
#include "rectangle.hpp"

extern "C" void app_main(void);

constexpr char TAG[] = "LCD";

using Color = Color565;
using LCD = ILI9341<Color>;

DMA_ATTR FrameBuffer<Color, LCD::ScreenSize> screenBuffer;

using Frame = LCD::Frame;

SPI spi{};
LCD lcd{};

void app_main(void)
{
	vTaskDelay(1000);

	spi = SPI{ SPI2_HOST, {GPIO_NUM_NC}, {GPIO_NUM_13}, {GPIO_NUM_14} };
	lcd = LCD{ spi, {GPIO_NUM_21}, {GPIO_NUM_47}, {GPIO_NUM_48}, &screenBuffer };

	lcd.init(Color::White);
	lcd.waitForDisplay();

	lcd.draw(Rectangle{ {1,1},{318,238},Color::Black });

	lcd.draw(Text{ { 1,1 }, std::is_same<LCD, ILI9341<Color666>>::value ? "RGB666" : "RGB565", Color::White, Color::Blue });
	lcd.draw(Line<Color>{ { 100, 0 }, { 100,239 }, Color::White });

	for (unsigned char i = 0; i < 64; i++)
	{
		lcd.draw((Pixel<Color>{{ i, 050 }, { i,0,0 }}));
		lcd.draw((Pixel<Color>{{ i, 100 }, { 0,i,0 }}));
		lcd.draw((Pixel<Color>{{ i, 150 }, { 0,0,i }}));
	}

	lcd.test();
	lcd.waitForDisplay();

	auto lastTime = clock();
	unsigned deltaTime = 0;
	unsigned count = 0;

	// 666 @ 40M : 46ms
	// 565 @ 40M : 30ms
	// 666 @ 80M~: 23ms
	// 565 @ 80M~: 15ms

	while (true)
	{
		vTaskDelay(1);
		lastTime = clock();
		lcd.display();
		lcd.waitForDisplay();
		deltaTime = clock() - lastTime;

		lcd.draw(Number<Color, unsigned>{ {10, 20}, deltaTime});

		lcd.draw(Rectangle{ { 10,200 }, { 9 + 8 * 4,215 }, Color::Black });
		lcd.draw(Number<Color, signed char>{ {10, 200}, (signed char)count});

		lcd.draw(Number<Color, unsigned>{ {10, 216}, count});
		count++;
	}
}
