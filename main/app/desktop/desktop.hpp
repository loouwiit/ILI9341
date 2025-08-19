#pragma once

#include "app.hpp"

class AppDesktop final : public App
{
public:
	AppDesktop(LCD& lcd, FT6X36& touch, ExitCallback_t exitCallback) : App(lcd, touch, exitCallback) {}

	virtual void init() override final;

	virtual void draw() override final;
	virtual void touchUpdate() override final;
	virtual void back() override final;

private:
	constexpr static Vector2s StaticOffset = { 30,50 };
	constexpr static unsigned short GapSize = 40;
	constexpr static unsigned short BlockSize = 100;
	constexpr static unsigned char TextSize = 2;

	constexpr static float moveThreshold2 = 100.0f;

	constexpr static unsigned char ApplicationSize = 3;

	constexpr const static char* ApplicationName[ApplicationSize] = { "touch","clock","setting" };

	LCD::Layar<LayarClassicSize::Middle> applications{ ApplicationSize * 2 };
	LCD::Rectangle applicationRectangle[ApplicationSize]{};
	LCD::Text applicationText[ApplicationSize]{};

	std::mutex clickMutex;
	short& offset = applications.start.x;
	bool fingerActive[2] = { false, false };
	Vector2s lastFingerPosition[2]{};
	Vector2s fingerMoveTotol[2]{};

	App* appFactory(unsigned char index);

	void click(Finger finger);
};
