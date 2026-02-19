#pragma once

#include "app/app.hpp"

#include "LCD/autoLanguage.hpp"

class AppAudio final : public App
{
public:
	AppAudio(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback) : App(lcd, touch, changeAppCallback, newAppCallback) {}

	virtual void init() override final;
	virtual void deinit() override final;

	virtual void draw() override final;
	virtual void touchUpdate() override final;
	virtual void back() override final { changeAppCallback(nullptr); }

private:
	constexpr static auto TAG = "AppAudio";

	constexpr static unsigned char TitleSize = 3;
	constexpr static unsigned char TextSize = 2;
	constexpr static unsigned char PauseSize = 4;
	constexpr static unsigned char ButtonSize = 3;
	constexpr static short ButtonBoardSize = 8;
	constexpr static short GapSize = 8;
	constexpr static short ContentXOffset = 20;
	constexpr static LCD::Color BackgroundColor = { 0x20,0x20,0x20 };

	constexpr static unsigned char ContensSize = 5;
	LCD::Layar<LayarClassicSize::Middle> contents{ ContensSize };
	LCD::Text title{ {LCD::ScreenSize.x / 2, 0}, AutoLnaguage{"audio", "音乐"}, TitleSize }; // 貌似翻译不太对的样子
	LCD::Text audioText{ {ContentXOffset, 16 * TitleSize + GapSize }, AutoLnaguage{"playing:", "当前播放:"}, TextSize, LCD::Color::White, BackgroundColor };
	char audioPathBuffer[256]{};
	LCD::Text audioFileText{ audioText.position + Vector2s{(short)audioText.computeSize().x, 0}, audioPathBuffer, TextSize, LCD::Color::White, BackgroundColor };

	LCD::Text pauseText{ Vector2s{(short)LCD::ScreenSize.x , (short)(LCD::ScreenSize.y + audioText.position.y + audioText.getSize().y)} / 2, "|>", PauseSize, LCD::Color::White, BackgroundColor };

	LCD::Layar<LayarClassicSize::Small> endButton{ {pauseText.position}, {(Vector2s)(fontsFullWidth(' ').size * ButtonSize)} ,2 }; // layer的size不稳定，初始化后数值不保证，或许以后会重构掉（因为修改start的时候不会同时修改end）；早期没有统一的element设计导致的
	LCD::Rectangle endButtonBackground{ {},endButton.getSize() , BackgroundColor };
	LCD::Rectangle endButtonFrontground{ endButtonBackground.start + Vector2s{ButtonBoardSize,ButtonBoardSize},endButtonBackground.getSize() - Vector2s{ButtonBoardSize,ButtonBoardSize} * 2, LCD::Color::White };

	void playAudio(const char* path);

	bool audioPaused{};
	bool audioOpened{};

	void updatePauseStatus();
	void switchPause();
	void pause();
	void resume();

	void end();

	Mutex deamonMutex{};
	static TickType_t deamonTask(void* param);
};
