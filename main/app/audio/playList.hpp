#pragma once

#include "app/app.hpp"

#include "LCD/autoLanguage.hpp"

class AppPlayList final : public App
{
public:
	AppPlayList(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback) : App(lcd, touch, changeAppCallback, newAppCallback) {}

	virtual void init() override final;
	virtual void focusIn() override final;
	virtual void deinit() override final;

	virtual void draw() override final;
	virtual void touchUpdate() override final;
	virtual void back() override final { running = false; changeAppCallback(nullptr); }

private:
	constexpr static auto TAG = "AppPlayList";

	constexpr static unsigned char TitleSize = 3;
	constexpr static unsigned char TextSize = 2;
	constexpr static short GapSize = 8;
	constexpr static short ContentXOffset = 20;
	constexpr static LCD::Color BackgroundColor = { 0x20,0x20,0x20 };

	constexpr static unsigned char ContensSize = 2;
	LCD::Layar<LayarClassicSize::Middle> contents{ ContensSize };
	LCD::Text title{ {LCD::ScreenSize.x / 2, 0}, AutoLnaguage{"play list", "播放列表"}, TitleSize };

	constexpr static size_t PlayListMaxSize = 50;
	LCD::Layar<LayarClassicSize::Huge> playListLayar{ {ContentXOffset, 16 * TitleSize + GapSize }, LCD::ScreenSize, 0 };
	LCD::Text playListText[PlayListMaxSize]{};
	AppPlayList* playListCallbackParam[PlayListMaxSize]{};

	void loadTexts();

	bool drawLocked = false;

	constexpr static float moveThreshold2 = 100.0f;
	constexpr static TickType_t holdTickThreshold = 500;

	TickType_t fingerHoldTick[2]{ (TickType_t)-1,(TickType_t)-1 };
	bool fingerActive[2] = { false, false };
	Vector2s lastFingerPosition[2]{};
	Vector2s fingerMoveTotol[2]{};

	void click(Finger finger);
	void releaseDetect();

	bool deamonRunning = false;
	Mutex deamonMutex{};
	static TickType_t deamonTask(void* param);

	void* audioPlayListPointer{};
};
