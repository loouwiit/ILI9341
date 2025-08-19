#include "setting.hpp"

#include <esp_log.h>

// #include "wifiSetting.hpp"
#include "systemInfo.hpp"

constexpr static char TAG[] = "AppSetting";

void AppSetting::init()
{
	App::init();

	title.position.x -= title.computeSize().x / 2;
	title.computeSize();
	title.clickCallbackParam = this;
	title.releaseCallback = [](Finger&, void* param) { AppSetting& self = *(AppSetting*)param; self.exitCallback(nullptr); };

	for (unsigned char i = 0; i < SettingSize; i++)
	{
		settings[i].text = SettingName[i];
		settings[i].textColor = LCD::Color::White;
		settings[i].backgroundColor = BackgroundColor;
		settings[i].scale = TextSize;
		contents[i] = &settings[i];
	}

	settings[0].position.x = ContentXOffset;
	settings[0].position.y = title.position.y + title.getSize().y + GapSize;
	for (unsigned char i = 1; i < SettingSize; i++)
	{
		settings[i].position.x = ContentXOffset;
		settings[i].position.y = settings[i - 1].position.y + settings[i - 1].computeSize().y + GapSize;
		settings[i].computeSize();
	}

	contents[SettingSize] = &title;
}

void AppSetting::draw()
{
	lcd.clear();
	lcd.draw(contents);
}

void AppSetting::touchUpdate()
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
		releaseDetect();
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
		releaseDetect();
	}

	if (fingerActive[0])
	{
		auto movement = finger[0].position - lastFingerPosition[0];
		fingerMoveTotol[0] += movement;
		offset += movement.y;
		lastFingerPosition[0] = finger[0].position;
	}
	if (fingerActive[1])
	{
		auto movement = finger[1].position - lastFingerPosition[1];
		fingerMoveTotol[1] += movement;
		offset += movement.y;
		lastFingerPosition[1] = finger[1].position;
	}
}

void AppSetting::back()
{
	exitCallback(nullptr);
}

App* AppSetting::appFactory(unsigned char index)
{
	switch (index)
	{
	// case 0:
	// 	return new WifiSetting{ lcd, touch, exitCallback };
	case 2:
		return new SystemInfo{ lcd, touch, exitCallback };
	default:
		ESP_LOGW(TAG, "failed to new setting %s (case %d)", SettingName[index], index);
		return nullptr;
	}
}

void AppSetting::click(Finger finger)
{
	finger.position.y -= offset;

	if (title.isClicked(finger.position))
	{
		clickMutex.lock();
		exitCallback(nullptr);
		return;
	}

	for (unsigned char i = 0; i < SettingSize;i++) if (settings[i].isClicked(finger.position))
	{
		if (!clickMutex.try_lock())
		{
			ESP_LOGI(TAG, "lock failed");
			return;
		}
		App* nextApp = appFactory(i);
		if (nextApp != nullptr) exitCallback(appFactory(i));
		else clickMutex.unlock();
	}
}

void AppSetting::releaseDetect()
{
	if (touch[0].state != Finger::State::Contact && touch[1].state != Finger::State::Contact)
	{
		if (offset > 0)
			offset = 0;
	}
}
