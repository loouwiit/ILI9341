#include "gpio.hpp"
#include "esp_attr.h"

IRAM_ATTR void gpioClearCallback(void* param)
{
	*(GPIO*)param = false;
}

IRAM_ATTR void gpioSetCallback(void* param)
{
	*(GPIO*)param = true;
}
