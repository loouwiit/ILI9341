#pragma once

#include "app.hpp"

class AppDesktop final : public App
{
public:
	AppDesktop(LCD& lcd, FT6X36& touch, ExitCallback_t exitCallback) : App(lcd, touch, exitCallback) {}

	virtual void init() override;

	virtual void draw() override;
	virtual void touchUpdate() override;
	virtual void back() override;

private:
	constexpr static Vector2s StaticOffset = { 30,50 };
	constexpr static unsigned short GapSize = 40;
	constexpr static unsigned short BlockSize = 100;
	constexpr static unsigned char TextSize = 2;

	constexpr static float moveThreshold2 = 100.0f;

	LCD::Layar<LayarClassicSize::Small> applications{ 4 };
	LCD::Rectangle applicationRectangle[2]{
		{StaticOffset + Vector2s{(BlockSize + GapSize) * 0,0}, {BlockSize,BlockSize}, LCD::Color::White},
		{StaticOffset + Vector2s{(BlockSize + GapSize) * 1,0}, {BlockSize,BlockSize}, LCD::Color::White}
	};
	LCD::Text applicationText[2]{
		{StaticOffset + Vector2s{(BlockSize + GapSize) * 0,0} + Vector2s{BlockSize / 2, BlockSize}, "touch", TextSize},
		{StaticOffset + Vector2s{(BlockSize + GapSize) * 1,0} + Vector2s{BlockSize / 2, BlockSize}, "clock", TextSize}
	};

	std::mutex clickMutex;
	short& offset = applications.start.x;
	bool fingerActive[2] = { false, false };
	Vector2s lastFingerPosition[2]{};
	Vector2s fingerMoveTotol[2]{};

	void click(Finger finger);
};
