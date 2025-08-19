#pragma once

#include "app.hpp"

class AppClock final : public App
{
public:
	AppClock(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback) : App(lcd, touch, changeAppCallback, changeAppCallback) {}

	virtual void init() override final;
	virtual void deinit() override final;

	virtual void draw() override final;
	virtual void back() override final;

private:
	char dateBuffer[11] = "2000/01/01";
	char timeBuffer[9] = "00:00:00";

	constexpr static Vector2s Offset = { 0, -8 };
	constexpr static unsigned short GapSize = 24;
	constexpr static unsigned char DateScale = 3;
	constexpr static unsigned char TimeScale = 4;

	LCD::Text dateText{ Offset + Vector2s{320 / 2 - 10 * 8 * DateScale / 2, 240 / 2 - 16 * DateScale / 2 - 16 * TimeScale / 2 - GapSize / 2}, dateBuffer, LCD::Color::White, LCD::Color::Black, DateScale };
	LCD::Text timeText{ Offset + Vector2s{320 / 2 - 8 * 8 * TimeScale / 2, 240 / 2 - 16 * DateScale / 2 - 16 * TimeScale / 2 + GapSize / 2 + 16 * DateScale}, timeBuffer, LCD::Color::White, LCD::Color::Black, TimeScale };

	void updateTime(time_t nowTime);
};
