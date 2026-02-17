#include "desktop.hpp"

#include <esp_log.h>

#include "app/setting/setting.hpp"
#include "app/clock/clock.hpp"
#include "app/tracker/tracker.hpp"
#include "app/server/server.hpp"
#include "app/explorer/explorer.hpp"
#include "app/tetris/tetris.hpp"
#include "app/strip/strip.hpp"
#include "app/audio/audio.hpp"

constexpr char TAG[] = "AppDesktop";

void AppDesktop::init()
{
	for (unsigned char i = 0; i < ApplicationSize; i++)
	{
		applicationRectangle[i] = { StaticOffset + Vector2s{(short)((BlockSize + GapSize) * i),0}, {BlockSize,BlockSize}, LCD::Color::White };

		applicationText[i] = { StaticOffset + Vector2s{(short)((BlockSize + GapSize) * i),0} + Vector2s{BlockSize / 2, BlockSize + TextGapSize}, ApplicationName[i], TextSize };
		applicationText[i].position.x -= applicationText[i].computeSize().x / 2;
		applicationText[i].computeSize();

		applications[i] = &applicationRectangle[i];
		applications[ApplicationSize + i] = &applicationText[i];
	}
	App::init();
}

void AppDesktop::focusIn()
{
	running = true;
	drawLocked = false;
}

void AppDesktop::draw()
{
	while (drawLocked && running) vTaskDelay(1);
	drawLocked = true;
	lcd.clear();
	lcd.draw(applications);
}

void AppDesktop::touchUpdate()
{
	Finger finger[2] = { touch[0],touch[1] };

	if (finger[0].state == Finger::State::Press)
	{
		fingerActive[0] = true;
		lastFingerPosition[0] = finger[0].position;
		fingerMoveTotol[0] = {};
	}
	else if (finger[0].state == Finger::State::Realease && fingerActive[0])
	{
		if (abs2(fingerMoveTotol[0]) < moveThreshold2)
			click(finger[0]);
		fingerActive[0] = false;
	}

	if (finger[1].state == Finger::State::Press)
	{
		fingerActive[1] = true;
		lastFingerPosition[1] = finger[1].position;
		fingerMoveTotol[1] = {};
	}
	else if (finger[1].state == Finger::State::Realease && fingerActive[1])
	{
		if (abs2(fingerMoveTotol[1]) < moveThreshold2)
			click(finger[1]);
		fingerActive[1] = false;
	}

	if (fingerActive[0])
	{
		auto movement = finger[0].position - lastFingerPosition[0];
		fingerMoveTotol[0] += movement;
		offset += movement.x;
		lastFingerPosition[0] = finger[0].position;
		if (movement.x != 0) drawLocked = false;
	}
	if (fingerActive[1])
	{
		auto movement = finger[1].position - lastFingerPosition[1];
		fingerMoveTotol[1] += movement;
		offset += movement.x;
		lastFingerPosition[1] = finger[1].position;
		if (movement.x != 0) drawLocked = false;
	}
}

void AppDesktop::back()
{
	offset = 0;
	lastFingerPosition[0] = touch[0].position;
	lastFingerPosition[1] = touch[1].position;
	fingerMoveTotol[0] = {};
	fingerMoveTotol[1] = {};
}

App* AppDesktop::appFactory(unsigned char index)
{
	switch (index)
	{
	case 0:
		return new AppSetting{ lcd, touch, changeAppCallback, newAppCallback };
	case 1:
		return new AppClock{ lcd, touch, changeAppCallback, newAppCallback };
	case 2:
		return new AppTracker{ lcd, touch, changeAppCallback, newAppCallback };
	case 3:
		return new AppServer{ lcd, touch, changeAppCallback, newAppCallback };
	case 4:
		return new AppExplorer{ lcd, touch, changeAppCallback, newAppCallback };
	case 5:
		return new AppTetris{ lcd, touch, changeAppCallback, newAppCallback };
	case 6:
		return new AppStrip{ lcd, touch, changeAppCallback, newAppCallback };
	case 7:
		return new AppAudio{ lcd, touch, changeAppCallback, newAppCallback };
	default:
		ESP_LOGE(TAG, "failed to new app %s (case %d)", ApplicationName[index].english, index);
		return nullptr;
	}
}

void AppDesktop::click(Finger finger)
{
	finger.position.x -= offset;

	for (unsigned char i = 0; i < ApplicationSize;i++) if (applicationRectangle[i].isClicked(finger.position))
	{
		ESP_LOGI(TAG, "start %s", ApplicationName[i].english);
		running = false;
		newAppCallback(appFactory(i));
	}
}
