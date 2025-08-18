#pragma once

#include "app.hpp"

class AppDesktop final : public App
{
public:
	AppDesktop(LCD& lcd, FT6X36& touch, ExitCallback_t exitCallback) : App(lcd, touch, exitCallback) {}

	virtual void init() override;

	virtual void draw() override;
	virtual void touchUpdate() override;

private:
	constexpr static Vector2s offset = { 30,50 };
	constexpr static unsigned short gapSize = 40;
	constexpr static unsigned short blockSize = 100;
	constexpr static unsigned char textSize = 2;

	LCD::Layar<LayarClassicSize::Small> applications{ 4 };
	LCD::Rectangle applicationRectangle[2]{
		{offset + Vector2s{(blockSize + gapSize) * 0,0}, {blockSize,blockSize}, LCD::Color::White},
		{offset + Vector2s{(blockSize + gapSize) * 1,0}, {blockSize,blockSize}, LCD::Color::White}
	};
	LCD::Text applicationText[2]{
		{offset + Vector2s{(blockSize + gapSize) * 0,0} + Vector2s{blockSize / 2, blockSize}, "touch", textSize},
		{offset + Vector2s{(blockSize + gapSize) * 1,0} + Vector2s{blockSize / 2, blockSize}, "clock", textSize}
	};

	std::mutex clickMutex;
};
