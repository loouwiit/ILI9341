#include <iostream>
#include <esp_log.h>

#include "ILI9341.hpp"
#include "FT6X36.hpp"

extern "C" void app_main(void);

constexpr char TAG[] = "LCD";

using LCD = ILI9341<Color565>;

DMA_ATTR LCD::Frame screenBuffer;

SPI spi{};
LCD lcd{};

IIC iic{};
FT6X36 touch{};

bool autoDisplayEnable = true;
void autoDisplay(void* param)
{
	if (!autoDisplayEnable) return;
	LCD& lcd = *(LCD*)param;
	lcd.display(autoDisplay, &lcd);
}

void app_main(void)
{
	spi = SPI{ SPI2_HOST, {GPIO_NUM_NC}, {GPIO_NUM_13}, {GPIO_NUM_14} };
	lcd = LCD{ spi, {GPIO_NUM_21}, {GPIO_NUM_47}, {GPIO_NUM_48}, &screenBuffer };

	lcd.init(LCD::Color::White);
	lcd.draw(LCD::Rectangle{ {1,1},{318,238},LCD::Color::Black });

	lcd.waitForDisplay();
	autoDisplay(&lcd);

	GPIO::enableGlobalInterrupt();
	LCD::Number<unsigned> interruptCount{ {250,100}, 0 };

	iic = IIC{ {GPIO_NUM_11}, {GPIO_NUM_12} };
	touch = FT6X36{ iic, {GPIO_NUM_10}, {GPIO_NUM_9} };

	touch.init();

	if (!touch.detectSelf())
	{
		lcd.draw(LCD::Text{ {10,10}, "touch not connected" });
		return;
	}

	LCD::Number<unsigned> number{ {10,10} };

	LCD::Number<unsigned> state[2]{
		{ {10,10 + 16 * 1},(unsigned)FT6X36::Finger::State::None },
		{ {10,10 + 16 * 2},(unsigned)FT6X36::Finger::State::None }
	};

	LCD::Layar<2> line1Clear{};
	LCD::Rectangle line1XClear{ {0,0},{320,1}, LCD::Color::Black };
	LCD::Rectangle line1YClear{ {0,0},{1,240}, LCD::Color::Black };
	line1Clear[0] = &line1XClear;
	line1Clear[1] = &line1YClear;

	LCD::Layar<2> line1{};
	LCD::Rectangle line1X{ {0,0},{320,1}, LCD::Color::Red };
	LCD::Rectangle line1Y{ {0,0},{1,240}, LCD::Color::Red };
	line1[0] = &line1X;
	line1[1] = &line1Y;


	LCD::Layar<2> line2Clear{};
	LCD::Rectangle line2XClear{ {0,0},{320,1}, LCD::Color::Black };
	LCD::Rectangle line2YClear{ {0,0},{1,240}, LCD::Color::Black };
	line2Clear[0] = &line2XClear;
	line2Clear[1] = &line2YClear;

	LCD::Layar<2> line2{};
	LCD::Rectangle line2X{ {0,0},{320,1}, LCD::Color::Blue };
	LCD::Rectangle line2Y{ {0,0},{1,240}, LCD::Color::Blue };
	line2[0] = &line2X;
	line2[1] = &line2Y;

	while (true)
	{
		if (touch.isNeedUpdate())
		{
			touch.update();
			number.number++;

			state[0].number = (unsigned)touch[0].state;
			state[1].number = (unsigned)touch[1].state;

			line1XClear.end.y = (line1XClear.start.y = line1X.start.y) + 1;
			line1YClear.end.x = (line1YClear.start.x = line1Y.start.x) + 1;
			line1X.end.y = (line1X.start.y = touch[0].position.y) + 1;
			line1Y.end.x = (line1Y.start.x = touch[0].position.x) + 1;

			line2XClear.end.y = (line2XClear.start.y = line2X.start.y) + 1;
			line2YClear.end.x = (line2YClear.start.x = line2Y.start.x) + 1;
			line2X.end.y = (line2X.start.y = touch[1].position.y) + 1;
			line2Y.end.x = (line2Y.start.x = touch[1].position.x) + 1;

			lcd.draw(number);
			lcd.draw(state[0]);
			lcd.draw(state[1]);
			lcd.draw(line1Clear);
			lcd.draw(line2Clear);
			if (state[0].number != 0x04)
				lcd.draw(line1);
			if (state[1].number != 0x04)
				lcd.draw(line2);
		}
		vTaskDelay(10);
	}
}
