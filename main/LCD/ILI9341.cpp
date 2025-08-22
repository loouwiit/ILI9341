#include "gpio.hpp"
#include "esp_attr.h"
#include "ILI9341.hpp"

ILI9341_IRAM void gpioClearCallback(void* param)
{
	*(GPIO*)param = false;
}

ILI9341_IRAM void gpioSetCallback(void* param)
{
	*(GPIO*)param = true;
}
