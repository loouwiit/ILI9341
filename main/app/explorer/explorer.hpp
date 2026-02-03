#pragma once

#include "app/app.hpp"
#include "LCD/autoLanguage.hpp"

#include "storge/fat.hpp"

class AppExplorer final : public App
{
public:
	AppExplorer(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback) : App(lcd, touch, changeAppCallback, newAppCallback) {}

	virtual void init() override final;
	virtual void deinit() override final;

	virtual void draw() override final;
	virtual void touchUpdate() override final;
	virtual void back() override final;

	void setTitle(const char* title);

	using CallbackFunction_t = void(*)(const char* path, void* param); // safe for changeAppCallback & newAppCallback
	CallbackFunction_t openFileCallback = nullptr;
	void* callBackParam = nullptr;

private:
	constexpr static unsigned char TitleSize = 3;
	constexpr static unsigned char TextSize = 2;
	constexpr static short GapSize = 8;
	constexpr static short ContentXOffset = 20;

	constexpr static LCD::Color FileColor = { 0xFF,0xFF,0xFF };
	constexpr static LCD::Color FloorColor = { 0,0,0xFF };
	constexpr static LCD::Color BackgroundColor = { 0x20,0x20,0x20 };

	constexpr static unsigned char ContensSize = 3;
	LCD::Layar<LayarClassicSize::Small> contents{ ContensSize };

	LCD::Text title{ {LCD::ScreenSize.x / 2, 0}, AutoLnaguage{ "explorer", "文件浏览" }, TitleSize };
	LCD::Text path{ {ContentXOffset, 16 * TitleSize + GapSize + (16 * TextSize + GapSize) * 0}, nullptr, TextSize, FloorColor, BackgroundColor };
	constexpr static unsigned char FileLayarSize = LayarClassicSize::Huge;
	LCD::Layar<FileLayarSize> fileLayar{ 0 };
	LCD::Text files[FileLayarSize]{};

	char* fileName[FileLayarSize]{};
	char realFloorPath[1024] = "/root/";
	char* const nowFloorPath = realFloorPath + 5;
	size_t nowFloorPointer = 1;
	Floor floor{};

	struct ClickCallbackParam_t { AppExplorer* self = nullptr; unsigned char index = 0; };
	ClickCallbackParam_t clickCallbackParam[FileLayarSize]{};

	void resetPosition();
	void updateFloor();
	bool floorBack();
	void clickCallback(unsigned char index);
	void openFile(unsigned char index);
	void updateText(unsigned char index, const char* text, Floor::Type type);

	constexpr static float moveThreshold2 = 100.0f;

	short& offset = contents.start.y;
	bool viewMoveActive[2] = { false, false };
	bool pathMoveActive[2] = { false, false };
	Vector2s lastFingerPosition[2]{};
	Vector2s fingerMoveTotol[2]{};

	void click(Finger finger);
	void releaseDetect();
};
