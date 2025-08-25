#pragma once

#include "app.hpp"

#include "storge/fat.hpp"

class AppTextEditor final : public App
{
public:
	AppTextEditor(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback) : App(lcd, touch, changeAppCallback, newAppCallback) {}

	virtual void init() override final;
	virtual void deinit() override final;

	virtual void draw() override final;
	virtual void touchUpdate() override final;
	virtual void back() override final;

	IOFile file{};
	const char* fileName = "";

private:
	constexpr static unsigned char TitleSize = 3;
	constexpr static unsigned char TextSize = 2;
	constexpr static short GapSize = 8;
	constexpr static short ContentXOffset = 20;

	constexpr static LCD::Color FileColor = { 63,63,63 };
	constexpr static LCD::Color FloorColor = { 0,0,63 };
	constexpr static LCD::Color BackgroundColor = { 8,8,8 };

	constexpr static unsigned char ContensSize = 2;
	LCD::Layar<LayarClassicSize::Pair> contents{ ContensSize };

	LCD::Text title{ {LCD::ScreenSize.x / 2, 0}, "error", TitleSize };

	constexpr static unsigned char LineCount = LayarClassicSize::Small;
	constexpr static size_t LineSize = 256;
	LCD::Layar<LineCount> fileLayar{ LineCount };
	LCD::Text lines[LineCount]{};
	char* lineBuffers[LineCount]{};
	FileBase::OffsetType fileOffset = 0;
	char* changeBuffer = nullptr;
	unsigned char changeIndex = 0;

	FileBase::OffsetType loadText(FileBase::OffsetType offset);
	void checkChange();

	struct ClickCallbackParam_t { AppTextEditor* self = nullptr; unsigned char index = 0; };
	ClickCallbackParam_t clickCallbackParam[LineCount]{};

	void clickCallback(unsigned char index);

	constexpr static float moveThreshold2 = 100.0f;

	Vector2s& offset = contents.start;
	bool fingerActive[2] = { false, false };
	Vector2s lastFingerPosition[2]{};
	Vector2s fingerMoveTotol[2]{};

	void click(Finger finger);
	void releaseDetect();
};
