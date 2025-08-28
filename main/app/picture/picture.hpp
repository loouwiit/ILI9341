#pragma once

#include "app.hpp"

#include <ctime>

#include "storge/fat.hpp"

class AppPicture final : public App
{
public:
	AppPicture(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback) : App(lcd, touch, changeAppCallback, newAppCallback) {}

	virtual void init() override final;
	// virtual void deinit() override final;

	virtual void draw() override final;
	virtual void touchUpdate() override final;
	virtual void back() override final;

	IFile file{};
	const char* fileName = "";

private:
	constexpr static auto mataDataSize = sizeof(unsigned short);
	constexpr static auto frameBufferSize = sizeof(LCD::Frame::buffer);
	constexpr static auto clockBufferSize = sizeof(clock_t);

	LCD::Text path{ { 0, (short)LCD::ScreenSize.y}, "error" };
	LCD::Number<unsigned short> pictureCountText{ { 0, (short)LCD::ScreenSize.y} };

	unsigned short& index = pictureCountText.number;
	unsigned short totol = 1;

	constexpr static clock_t TextFadeTime = 1000;
	clock_t textTime = (clock_t)-1;

	constexpr static clock_t frameTime = 100;
	clock_t nextFrameTime = 0;

	void tryLoadNext();
	void load(unsigned short index);

	void drawText();
};
