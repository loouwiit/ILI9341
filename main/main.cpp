#include <iostream>
#include <esp_log.h>

#include "ILI9341.hpp"
#include "iic.hpp"

extern "C" void app_main(void);

constexpr char TAG[] = "LCD";

using LCD = ILI9341<Color565>;

DMA_ATTR LCD::Frame screenBuffer;

SPI spi{};
LCD lcd{};

IIC iic{};

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
	GPIO reset{ GPIO_NUM_10,GPIO::Mode::GPIO_MODE_OUTPUT };
	GPIO interrupt{ GPIO_NUM_9, GPIO::Mode::GPIO_MODE_INPUT, GPIO::Pull::GPIO_PULLUP_ONLY, GPIO::Interrupt::GPIO_INTR_NEGEDGE,
		 [](void* param)
		 {
			unsigned& number = *(unsigned*)param;
			number++;
		 }, &interruptCount.number };

	reset = false;
	vTaskDelay(1);
	reset = true;
	vTaskDelay(1);

	LCD::Number <uint16_t> addressShape{ {10,10}, 0, 16 };
	auto& position = addressShape.position;
	auto& address = addressShape.number;
	for (; address <= 0x7F; address++)
	{
		addressShape.textColor = iic.detect(address) ? LCD::Color::Red : LCD::Color::White;

		lcd.draw(addressShape);

		position.y += 16;
		if (position.y >= 220)
		{
			position.y = 10;
			position.x += 24;
		}
		vTaskDelay(5);
	}

	while (true)
	{
		lcd.draw(interruptCount);
		vTaskDelay(10);
	}
}
