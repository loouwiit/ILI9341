#pragma once

#define defineString(x) #x

#include "app.hpp"

class SystemInfo final : public App
{
public:
	SystemInfo(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback) : App(lcd, touch, changeAppCallback, newAppCallback) {}

	virtual void init() override final;
	virtual void deinit() override final;

	virtual void draw() override final;
	virtual void touchUpdate() override final;
	virtual void back() override final;

private:
	constexpr static unsigned char TitleSize = 3;
	constexpr static unsigned char TextSize = 2;
	constexpr static unsigned char TaskTextSize = 1;
	constexpr static short GapSize = 8;
	constexpr static short ContentXOffset = 20;
	constexpr static LCD::Color BackgroundColor = { 8,8,8 };

	constexpr static unsigned char SettingSize = 10;

	constexpr static size_t TaskListBufferSize = 1024;

	char socBuffer[19] = "soc:" CONFIG_IDF_TARGET "@" "000MHz";
	char ramBuffer[19] = "ram:-1KB";
	char psramBuffer[19] = "psram:-1KB";
	char* taskListBuffer = nullptr;
	const char* SettingName[SettingSize] = { socBuffer, "flash size:" CONFIG_ESPTOOLPY_FLASHSIZE, ramBuffer, psramBuffer, "", "print to uart", "error in heap trace", "restart device", "", taskListBuffer };

	LCD::Layar<LayarClassicSize::Large> contents{ 1 + SettingSize };
	LCD::Text title{ {LCD::ScreenSize.x / 2, 0}, "system info", TitleSize };
	LCD::Text settings[SettingSize]{};

	constexpr static float moveThreshold2 = 100.0f;

	short& offset = contents.start.y;
	bool fingerActive[2] = { false, false };
	Vector2s lastFingerPosition[2]{};
	Vector2s fingerMoveTotol[2]{};

	void click(Finger finger);
	void releaseDetect();
	static void updateHeapTraceText(LCD::Text& text);

	static void printInfo();
};
