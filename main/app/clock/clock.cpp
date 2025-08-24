#include "clock.hpp"

#include <esp_log.h>
#include <esp_task.h>

constexpr char TAG[] = "AppClock";

void AppClock::init()
{
	auto nowTime = time(nullptr);
	ESP_LOGI(TAG, "init at %s", asctime(localtime(&nowTime)));

	App::init();

	if (xTaskCreate(
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
	, "appClock", 4096, this, 1, nullptr) != pdTRUE)
	{
		dateText.text = "error:out of memory";
		timeText.text = "error:out of memory";
		deleteAble = true;
	}
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

void AppClock::touchUpdate()
{
	if ((touch[0].state == Finger::State::Press && touch[1].state == Finger::State::Contact) ||
		(touch[1].state == Finger::State::Press && touch[0].state == Finger::State::Contact) ||
		(touch[0].state == Finger::State::Contact && touch[1].state == Finger::State::Contact))
		changeAppCallback(nullptr);
}

void AppClock::back()
{
	changeAppCallback(nullptr);
}

void AppClock::updateTime(time_t nowTime)
{
	tm* tm = localtime(&nowTime);

	strftime(dateBuffer, sizeof(dateBuffer), "%Y/%m/%d", tm);
	strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", tm);
}
