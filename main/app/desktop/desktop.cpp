#include "desktop.hpp"

#include "clock/clock.hpp"
#include "touch/touch.hpp"

constexpr char TAG[] = "AppDesktop";

void AppDesktop::init()
{
	applications[0] = &applicationRectangle[0];
	applications[1] = &applicationRectangle[1];
	applications[2] = &applicationText[0];
	applications[3] = &applicationText[1];

	applicationText[0].computeSize();
	applicationText[0].position.x -= applicationText[0].getSize().x / 2;
	applicationText[0].computeSize();

	applicationText[1].computeSize();
	applicationText[1].position.x -= applicationText[0].getSize().x / 2;
	applicationText[1].computeSize();
}

void AppDesktop::draw()
{
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
	else if (finger[0].state == Finger::State::Realease)
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
	else if (finger[1].state == Finger::State::Realease)
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
	}
	if (fingerActive[1])
	{
		auto movement = finger[1].position - lastFingerPosition[1];
		fingerMoveTotol[1] += movement;
		offset += movement.x;
		lastFingerPosition[1] = finger[1].position;

	}

	if (applicationRectangle[0].start.x + offset < 0)
		applicationRectangle[0].color = LCD::Color::Red;
	else if (applicationRectangle[0].start.x + offset > LCD::ScreenSize.x - BlockSize)
		applicationRectangle[0].color = LCD::Color::Blue;
	else applicationRectangle[0].color = LCD::Color::White;

	if (applicationRectangle[1].start.x + offset < 0)
		applicationRectangle[1].color = LCD::Color::Red;
	else if (applicationRectangle[1].start.x + offset > LCD::ScreenSize.x - BlockSize)
		applicationRectangle[1].color = LCD::Color::Blue;
	else applicationRectangle[1].color = LCD::Color::White;
}

void AppDesktop::back()
{
	offset = 0;
	lastFingerPosition[0] = touch[0].position;
	lastFingerPosition[1] = touch[1].position;
	fingerMoveTotol[0] = {};
	fingerMoveTotol[1] = {};
}

void AppDesktop::click(Finger finger)
{
	finger.position.x -= offset;

	if (applicationRectangle[0].isClicked(finger.position))
	{
		if (!clickMutex.try_lock())
		{
			ESP_LOGI(TAG, "lock failed");
			return;
		}

		ESP_LOGI(TAG, "change to Touch");
		exitCallback(new AppTouch{ lcd, touch, exitCallback });
	}

	if (applicationRectangle[1].isClicked(finger.position))
	{
		if (!clickMutex.try_lock())
		{
			ESP_LOGI(TAG, "lock failed");
			return;
		}

		ESP_LOGI(TAG, "change to Clock");
		exitCallback(new AppClock{ lcd, touch, exitCallback });
	}
}
