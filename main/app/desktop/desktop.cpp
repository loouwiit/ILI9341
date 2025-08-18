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

	applicationRectangle[0].clickCallbackParam = this;
	applicationRectangle[0].pressCallback = [](Finger&, void* param)
		{
			AppDesktop& self = *(AppDesktop*)param;
			if (!self.clickMutex.try_lock()) { ESP_LOGI(TAG, "lock failed"); return; }
			ESP_LOGI(TAG, "change to Touch");
			self.exitCallback(new AppTouch{ self.lcd, self.touch, self.exitCallback });
		};

	applicationRectangle[1].clickCallbackParam = this;
	applicationRectangle[1].pressCallback = [](Finger&, void* param)
		{
			AppDesktop& self = *(AppDesktop*)param;
			if (!self.clickMutex.try_lock()) { ESP_LOGI(TAG, "lock failed"); return; }
			ESP_LOGI(TAG, "change to Clock");
			self.exitCallback(new AppClock{ self.lcd, self.touch, self.exitCallback });
		};
}

void AppDesktop::draw()
{
	lcd.draw(applications);
}

void AppDesktop::touchUpdate()
{
	Finger finger[2] = { touch[0],touch[1] };
	applications.finger(finger[0]);
	applications.finger(finger[1]);
}
