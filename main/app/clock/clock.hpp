#pragma once

#include "app.hpp"

class AppClock final : public App
{
public:
	AppClock(LCD& lcd, FT6X36& touch, ExitCallback_t exitCallback) : App(lcd, touch, exitCallback) {}

	virtual void init() override;
	virtual void deinit() override;

	virtual void draw() override;
	virtual void back() override;

private:
	char dateBuffer[11] = "2000/01/01";
	char timeBuffer[9] = "00:00:00";

	constexpr static Vector2us Offset = { 0, (unsigned short)-8 };
	constexpr static unsigned short GapSize = 24;
	constexpr static unsigned char DateScale = 3;
	constexpr static unsigned char TimeScale = 4;

	LCD::Text dateText{ Offset + Vector2us{320 / 2 - 10 * 8 * DateScale / 2, 240 / 2 - 16 * DateScale / 2 - 16 * TimeScale / 2 - GapSize / 2}, dateBuffer, LCD::Color::White, LCD::Color::Black, DateScale };
	LCD::Text timeText{ Offset + Vector2us{320 / 2 - 8 * 8 * TimeScale / 2, 240 / 2 - 16 * DateScale / 2 - 16 * TimeScale / 2 + GapSize / 2 + 16 * DateScale}, timeBuffer, LCD::Color::White, LCD::Color::Black, TimeScale };

	void updateTime(time_t nowTime);
};
