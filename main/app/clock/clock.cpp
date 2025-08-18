#include "clock.hpp"

#include <esp_log.h>
#include <esp_task.h>

constexpr char TAG[] = "AppClock";

void AppClock::init()
{
	auto nowTime = time(nullptr);
	ESP_LOGI(TAG, "init at %s", asctime(localtime(&nowTime)));

	App::init();

	xTaskCreate(
		[](void* param)
		{
			AppClock& appClock = *(AppClock*)param;
			time_t lastTime = 0;
			time_t nowTime = 0;
			while (appClock.running)
			{
				vTaskDelay(10);
				nowTime = time(nullptr);
				if (nowTime == lastTime)
					continue;

				appClock.updateTime(nowTime);
				lastTime = nowTime;
			}
			appClock.deleteAble = true;
			ESP_LOGI(TAG, "deinit at %s", asctime(localtime(&nowTime)));
			vTaskDelete(nullptr);
		}
	, "appClock", 2048, this, 1, nullptr);
}

void AppClock::deinit()
{
	running = false;
}

void AppClock::draw()
{
	lcd.draw(dateText);
	lcd.draw(timeText);
}

void AppClock::updateTime(time_t nowTime)
{
	tm* tm = localtime(&nowTime);
	sprintf(dateBuffer, "%.4d/%.2d/%.2d", tm->tm_year, tm->tm_mon, tm->tm_mday);
	sprintf(timeBuffer, "%.2d:%.2d:%.2d", tm->tm_hour, tm->tm_min, tm->tm_sec);
}
