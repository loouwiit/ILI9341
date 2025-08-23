#pragma once

#include "app.hpp"

#include <ctime>

class TimeSetting final : public App
{
public:
	TimeSetting(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback) : App(lcd, touch, changeAppCallback, newAppCallback) {}

	virtual void init() override final;
	virtual void deinit() override final;

	virtual void draw() override final;
	virtual void touchUpdate() override final;
	virtual void back() override final;

private:
	constexpr static unsigned char TitleSize = 3;
	constexpr static unsigned char TextSize = 2;
	constexpr static short GapSize = 8;
	constexpr static short ContentXOffset = 20;
	constexpr static LCD::Color BackgroundColor = { 8,8,8 };

	constexpr static unsigned char ContensSize = 4;
	LCD::Layar<LayarClassicSize::Small> contents{ ContensSize };
	LCD::Text title{ {LCD::ScreenSize.x / 2, 0}, "time setting", TitleSize };
	LCD::Text nowDate{ {ContentXOffset, 16 * TitleSize + GapSize + (16 * TextSize + GapSize) * 0}, "error in date", TextSize, LCD::Color::White, BackgroundColor };
	LCD::Text nowTime{ {ContentXOffset, 16 * TitleSize + GapSize + (16 * TextSize + GapSize) * 1}, "error in time", TextSize, LCD::Color::White, BackgroundColor };
	LCD::Text ntpUpdate{ {ContentXOffset, 16 * TitleSize + GapSize + (16 * TextSize + GapSize) * 2}, "ntp update time", TextSize, LCD::Color::White, BackgroundColor };

	char dateBuffer[11] = "2000/01/01";
	char timeBuffer[9] = "00:00:00";

	void updateTime(time_t nowTime);

	Mutex syncMutex;
	bool syncing = false;

	constexpr static float moveThreshold2 = 100.0f;

	short& offset = contents.start.y;
	bool fingerActive[2] = { false, false };
	Vector2s lastFingerPosition[2]{};
	Vector2s fingerMoveTotol[2]{};

	void click(Finger finger);
	void releaseDetect();
};
