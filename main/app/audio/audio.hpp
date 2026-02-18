#pragma once

#include "app/app.hpp"

#include "LCD/autoLanguage.hpp"

class AppAudio final : public App
{
public:
	AppAudio(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback) : App(lcd, touch, changeAppCallback, newAppCallback) {}

	virtual void init() override final;
	// virtual void deinit() override final;

	virtual void draw() override final;
	virtual void touchUpdate() override final;
	virtual void back() override final { changeAppCallback(nullptr); }

private:
	constexpr static auto TAG = "AppAudio";

	constexpr static unsigned char TitleSize = 3;
	constexpr static unsigned char TextSize = 2;
	constexpr static short GapSize = 8;
	constexpr static short ContentXOffset = 20;
	constexpr static LCD::Color BackgroundColor = { 0x20,0x20,0x20 };
	constexpr static short BoardSize = 3;
	constexpr static TickType_t LastTimeStep = 10;

	constexpr static unsigned char ContensSize = 4;
	LCD::Layar<LayarClassicSize::Middle> contents{ ContensSize };
	LCD::Text title{ {LCD::ScreenSize.x / 2, 0}, AutoLnaguage{"audio", "音乐"}, TitleSize }; // 貌似翻译不太对的样子
	LCD::Text audioText{ {ContentXOffset, 16 * TitleSize + GapSize + (16 * TextSize + GapSize) * 0}, AutoLnaguage{"playing:", "当前播放:"}, TextSize, LCD::Color::White, BackgroundColor };
	char audioFileBuffer[256]{};
	LCD::Text audioFileText{ audioText.position + Vector2s{(short)audioText.computeSize().x, 0}, audioFileBuffer, TextSize, LCD::Color::White, BackgroundColor };

	LCD::Text pauseText{ audioText.position + Vector2s{0,(short)(audioText.computeSize().y + GapSize)}, "pause:error", TextSize, LCD::Color::White, BackgroundColor };

	void playAudio(const char* path);
	void switchPause();
	void pause();
	void resume();
};
