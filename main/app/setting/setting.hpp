#pragma once

#include "app.hpp"

class AppSetting final : public App
{
public:
	AppSetting(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback) : App(lcd, touch, changeAppCallback, newAppCallback) {}

	virtual void init() override final;

	virtual void draw() override final;
	virtual void touchUpdate() override final;
	virtual void back() override final;

private:
	constexpr static unsigned char TitleSize = 3;
	constexpr static unsigned char TextSize = 2;
	constexpr static short GapSize = 8;
	constexpr static short ContentXOffset = 20;
	constexpr static LCD::Color BackgroundColor = { 8,8,8 };

	constexpr static unsigned char SettingSize = 9;
	constexpr static const char* SettingName[SettingSize] = { "wifi setting","time setting", "system info", "touch test", "", "--debug--", "just a very \nlong setting", "nop1", "nop2"};

	LCD::Layar<LayarClassicSize::Middle> contents{ 1 + SettingSize };
	LCD::Text title{ {LCD::ScreenSize.x / 2, 0}, "setting", TitleSize };
	LCD::Text settings[SettingSize]{};

	constexpr static float moveThreshold2 = 100.0f;

	Mutex exitMutex;
	bool exiting = false;
	short& offset = contents.start.y;
	bool fingerActive[2] = { false, false };
	Vector2s lastFingerPosition[2]{};
	Vector2s fingerMoveTotol[2]{};

	App* appFactory(unsigned char index);

	void click(Finger finger);
	void releaseDetect();
};
