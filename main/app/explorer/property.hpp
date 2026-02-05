#pragma once

#include "app/app.hpp"
#include "LCD/autoLanguage.hpp"

class AppProperty final : public App
{
public:
	AppProperty(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback) : App(lcd, touch, changeAppCallback, newAppCallback) {}

	virtual void init() override final;
	virtual void deinit() override final;

	virtual void draw() override final;
	virtual void touchUpdate() override final;
	virtual void back() override final;

	void setTitle(const char* title);
	void setTitleBuffer(const char* titleBuffer);

	void setPath(const char* path);

private:
	constexpr static char TAG[] = "AppProperty";

	constexpr static unsigned char TitleSize = 3;
	constexpr static unsigned char TextSize = 2;
	constexpr static short GapSize = 8;
	constexpr static short ContentXOffset = 20;

	constexpr static LCD::Color FileColor = { 0xFF,0xFF,0xFF };
	constexpr static LCD::Color FloorColor = { 0,0,0xFF };
	constexpr static LCD::Color BackgroundColor = { 0x20,0x20,0x20 };

	constexpr static unsigned char ContensSize = 6;
	LCD::Layar<LayarClassicSize::Middle> contents{ ContensSize };

	const char* titleBuffer = nullptr;
	const char* defaultTitle = AutoLnaguage{ "property", "属性" };
	LCD::Text title{ {LCD::ScreenSize.x / 2, 0}, defaultTitle, TitleSize };

	char pathBuffer[256]{};
	char* const nowFloorPath = pathBuffer + 5;
	LCD::Layar<LayarClassicSize::Pair> pathLayar{ {ContentXOffset, 16 * TitleSize + GapSize + (16 * TextSize + GapSize) * 0}, {(short)(LCD::ScreenSize.x), 16 * TitleSize}, 2 };
	LCD::Text pathDescripText{ {}, AutoLnaguage{ "path:", "路径:" }, TextSize, LCD::Color::White, BackgroundColor };
	LCD::Text pathText{ {(short)pathDescripText.computeSize().x, 0}, nowFloorPath, TextSize, LCD::Color::White, BackgroundColor };

	LCD::Text typeText{ {ContentXOffset, 16 * TitleSize + GapSize + (16 * TextSize + GapSize) * 1}, AutoLnaguage{ "type:error", "类型:错误" }, TextSize, LCD::Color::White, BackgroundColor };

	LCD::Text sizeDescripText{ {ContentXOffset, 16 * TitleSize + GapSize + (16 * TextSize + GapSize) * 2} , AutoLnaguage{ "size:", "大小:" }, TextSize, LCD::Color::White, BackgroundColor };
	LCD::Number<size_t> sizeText{ sizeDescripText.position + Vector2s{(short)sizeDescripText.computeSize().x, 0}, (size_t)(-1), 10, LCD::Color::White, BackgroundColor, TextSize };

	LCD::Text deleteText{ {ContentXOffset, (short)LCD::ScreenSize.y} , AutoLnaguage{ "delete", "删除" }, TextSize, LCD::Color::Red, BackgroundColor };

	constexpr static float moveThreshold2 = 100.0f;

	bool fingerActive[2] = { false, false };
	bool fingerActiveMovePath[2] = { false, false };
	Vector2s lastFingerPosition[2]{};
	Vector2s fingerMoveTotol[2]{};

	void click(Finger finger);
	void releaseDetect();
};
