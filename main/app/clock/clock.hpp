#pragma once

#include "app.hpp"

class AppClock final : public App
{
public:
	AppClock(LCD& lcd, FT6X36& touch, ExitCallback_t exitCallback) : App(lcd, touch, exitCallback) {}

	virtual void init() override;
	virtual void deinit() override;

	virtual void draw() override;
	virtual void touchUpdate() override {};

private:
	char dateBuffer[11] = "2000/01/01";
	char timeBuffer[9] = "00:00:00";
	LCD::Text dateText{ {320 / 2 - 10 * 8 / 2, 100}, dateBuffer };
	LCD::Text timeText{ {320 / 2 - 8 * 8 / 2, 116}, timeBuffer };

	void updateTime(time_t nowTime);
};
