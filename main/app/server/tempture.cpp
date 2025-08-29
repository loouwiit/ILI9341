#include <driver/temperature_sensor.h>
#include "mutex.hpp"
#include "tempture.hpp"

EXT_RAM_BSS_ATTR static temperature_sensor_handle_t temp_handle = nullptr;
EXT_RAM_BSS_ATTR static bool tempratureInited = false;
EXT_RAM_BSS_ATTR static bool tempratureEnabled = false;
Mutex temperatureMutex;

bool temperatureIsInited()
{
	return tempratureInited;
}

bool temperatureIsStarted()
{
	return tempratureEnabled;
}

void temperatureInit()
{
	if (tempratureInited) return;
	temperature_sensor_config_t temp_sensor{};
	temp_sensor.range_min = -10;
	temp_sensor.range_max = 80;
	ESP_ERROR_CHECK(temperature_sensor_install(&temp_sensor, &temp_handle));
	tempratureInited = true;
}

void temperatureDeinit()
{
	if (!tempratureInited) return;
	temperature_sensor_uninstall(temp_handle);
	tempratureInited = false;
}

void temperatureStart()
{
	if (!tempratureInited) return;
	if (tempratureEnabled) return;
	ESP_ERROR_CHECK(temperature_sensor_enable(temp_handle));
	tempratureEnabled = true;
}

void temperatureStop()
{
	if (!tempratureInited) return;
	if (!tempratureEnabled) return;
	tempratureEnabled = false;
	ESP_ERROR_CHECK(temperature_sensor_disable(temp_handle));
}

float temperatureGet()
{
	if (!tempratureInited) return FLT_MAX;
	float temperature = 0.0f;
	if (!tempratureEnabled) return FLT_MAX;
	if (!temperatureMutex.try_lock()) return FLT_MAX;
	auto ret = temperature_sensor_get_celsius(temp_handle, &temperature);
	temperatureMutex.unlock();
	if (ret == ESP_OK)
		return temperature;
	else
	{
		printf("temperature: get temperature error: %d\n\ttemperature is %f\n", ret, temperature);
		ESP_ERROR_CHECK(ret);
		return FLT_MAX;
	}
}
