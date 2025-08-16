#include <iostream>
#include <esp_log.h>

#include "ILI9341.hpp"

extern "C" void app_main(void);

constexpr char TAG[] = "LCD";

using LCD = ILI9341<Color565>;

DMA_ATTR LCD::Frame screenBuffer;

SPI spi{};
LCD lcd{};

void app_main(void)
{
	vTaskDelay(1000);

	spi = SPI{ SPI2_HOST, {GPIO_NUM_NC}, {GPIO_NUM_13}, {GPIO_NUM_14} };
	lcd = LCD{ spi, {GPIO_NUM_21}, {GPIO_NUM_47}, {GPIO_NUM_48}, &screenBuffer };

	lcd.init(LCD::Color::White);
	lcd.waitForDisplay();

	lcd.draw(LCD::Rectangle{ {1,1},{318,238},LCD::Color::Black });

	lcd.draw(LCD::Text{ { 1,1 }, std::is_same<LCD, ILI9341<Color666>>::value ? "RGB666" : "RGB565", LCD::Color::White, LCD::Color::Blue });
	lcd.draw(LCD::Line{ { 100, 0 }, { 100,239 }, LCD::Color::White });

	for (unsigned char i = 0; i < 64; i++)
	{
		lcd.draw((LCD::Pixel{ { i, 050 }, { i,0,0 } }));
		lcd.draw((LCD::Pixel{ { i, 100 }, { 0,i,0 } }));
		lcd.draw((LCD::Pixel{ { i, 150 }, { 0,0,i } }));
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

	LCD::Rectangle number1Eraser{ { 10,200 }, { 9 + 8 * 4,215 }, LCD::Color::Black };
	LCD::Number<signed char> number1{ {10, 200}, (signed char)count };
	LCD::Number<unsigned>number2{ {10, 216}, count };

	LCD::Layar<3> countLayer{};
	countLayer[0] = &number1Eraser;
	countLayer[1] = &number1;
	countLayer[2] = &number2;

	while (true)
	{
		vTaskDelay(1);
		lastTime = clock();
		lcd.display();
		lcd.waitForDisplay();
		deltaTime = clock() - lastTime;

		lcd.draw(LCD::Number<unsigned> { {10, 20}, deltaTime });

		number1.number = (signed char)count;
		number2.number = count;

		lcd.draw(countLayer);
		count++;
	}
}
