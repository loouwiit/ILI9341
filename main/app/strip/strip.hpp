#pragma once

#include "app.hpp"
#include "LCD/autoLanguage.hpp"

class AppStrip final : public App
{
public:
	AppStrip(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback) : App(lcd, touch, changeAppCallback, newAppCallback) {}

	virtual void init() override final;
	// virtual void focusIn() override final;
	// virtual void deinit() override final;

	virtual void draw() override final;
	virtual void touchUpdate() override final;
	virtual void back() override final;

private:
	constexpr static unsigned char TitleSize = 3;
	constexpr static unsigned char TextSize = 2;
	constexpr static short GapSize = 8;
	constexpr static short ContentXOffset = 20;
	constexpr static LCD::Color BackgroundColor = { 0x20,0x20,0x20 };
	constexpr static short BoardSize = 3;

	constexpr static auto gpioNum = GPIO_NUM_1;
	constexpr static uint32_t ledCount = 15;

	constexpr static unsigned char ContensSize = 3;
	LCD::Layar<LayarClassicSize::Small> contents{ ContensSize };
	LCD::Text title{ {LCD::ScreenSize.x / 2, 0}, AutoLnaguage{"strip", "灯带"}, TitleSize };
	LCD::Text stripText{ {ContentXOffset, 16 * TitleSize + GapSize + (16 * TextSize + GapSize) * 0}, AutoLnaguage{"strip:error", "灯带：错误"}, TextSize, LCD::Color::White, BackgroundColor };
	LCD::Layar<LayarClassicSize::Huge> ledLayar{ ledCount * 2 };
	LCD::Rectangle ledBoards[ledCount]{};
	LCD::Rectangle leds[ledCount]{};
	AppStrip* ledParams[ledCount]{};

	void updateState();

	App* appColorInput = nullptr;

	constexpr static float moveThreshold2 = 100.0f;

	bool fingerActive[2] = { false, false };
	bool fingerMoveLeds[2]{};
	Vector2s lastFingerPosition[2]{};
	Vector2s fingerMoveTotol[2]{};

	void click(Finger finger);
	void releaseDetect();
};
