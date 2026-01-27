#include "clock.hpp"

#include <esp_log.h>
#include <esp_task.h>

#include "task.hpp"

constexpr char TAG[] = "AppClock";

void AppClock::init()
{
	auto nowTime = time(nullptr);
	ESP_LOGI(TAG, "init at %s", asctime(localtime(&nowTime)));

	App::init();

	Task::addTask([](void* param) -> TickType_t
		{
			AppClock& appClock = *(AppClock*)param;
			time_t nowTime = time(nullptr);

			if (!appClock.running)
			{
				appClock.deleteAble = true;
				ESP_LOGI(TAG, "deinit at %s", asctime(localtime(&nowTime)));
				return Task::infinityTime;
			}

			if (nowTime != appClock.showTime)
				appClock.updateTime(nowTime);
			return 10;
		}, "appClock", this);
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
	ESP_LOGI(TAG, "try exit at %s", asctime(localtime(&showTime)));
	changeAppCallback(nullptr);
}

void AppClock::updateTime(time_t nowTime)
{
	showTime = nowTime;
	tm* tm = localtime(&nowTime);

	strftime(dateBuffer, sizeof(dateBuffer), "%Y/%m/%d", tm);
	strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", tm);
}
