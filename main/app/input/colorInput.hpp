#pragma once

#include "app.hpp"
#include "LCD/bar.hpp"

class AppColorInput final : public App
{
public:
	AppColorInput(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback) : App(lcd, touch, changeAppCallback, newAppCallback) {}

	virtual void init() override final;
	// virtual void deinit() override final;

	virtual void draw() override final;
	virtual void touchUpdate() override final;
	virtual void back() override final;

	static void defaultCallback(void*) {}

	void* callbackParam = nullptr;
	void (*finishCallback)(void*) = defaultCallback;
	void (*changeCallback)(void*) = defaultCallback;

	Color888 getColor()
	{
		return color;
	}

	void setColor(Color888 color)
	{
		AppColorInput::color = color;
		colorPreview.color = color;
		bar[0].setValue(color.R);
		bar[1].setValue(color.G);
		bar[2].setValue(color.B);
	}

private:
	constexpr static Vector2s ColorPreviewPosition{ 30,30 };
	constexpr static Vector2s ColorPreviewSize{ 260,40 };
	constexpr static short ColorPreviewBoardSize = 3;
	constexpr static Vector2s BarGap{ 0,45 };
	constexpr static Vector2s BarFirstPosition = Vector2s{ 15,200 } - BarGap * 2;
	constexpr static LCD::Color BackgroundColor = { 0x20,0x20,0x20 };

	Color888 color{};
	LCD::Layar<LayarClassicSize::Small>content{ 3 };

	LCD::Rectangle colorPreview{ ColorPreviewPosition,ColorPreviewSize, color };
	LCD::Rectangle colorPreviewBoard{ ColorPreviewPosition - Vector2s{ColorPreviewBoardSize,ColorPreviewBoardSize},ColorPreviewSize + Vector2s{ColorPreviewBoardSize,ColorPreviewBoardSize} * 2, BackgroundColor };
	LCD::Layar<LayarClassicSize::Small>bars{ 3 };

	LCD::Bar<uint8_t> bar[3]{ {{}, 0xFF},{{}, 0xFF},{{}, 0xFF} };
};
