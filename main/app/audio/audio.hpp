#pragma once

#include "app/app.hpp"
#include <ctime>

#warning developing here

class AppAudio final : public App
{
public:
	AppAudio(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback) : App(lcd, touch, changeAppCallback, changeAppCallback) {}

	virtual void init() override final;
	// virtual void deinit() override final;

	virtual void draw() override final {}
	virtual void touchUpdate() override final { back(); }
	virtual void back() override final { changeAppCallback(nullptr); }

private:
	constexpr static auto TAG = "AppAudio";
};
