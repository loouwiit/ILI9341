#pragma once

#include "app.hpp"
#include "socketStream.hpp"

class AppTracker final : public App
{
public:
	AppTracker(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback) : App(lcd, touch, changeAppCallback, changeAppCallback) {}

	virtual void init() override final;
	virtual void deinit() override final;

	virtual void draw() override final;
	virtual void touchUpdate() override final;
	virtual void back() override final;

private:
	char buffer[512] = "error in init";
	constexpr static unsigned char textSize = 2;
	LCD::Text text{ LCD::ScreenSize / 2, buffer, textSize };

	void update(const char* string);
	void updatePosition();
	void dealSocket(IOSocketStream& socketStream);

	constexpr static unsigned short Port = 467;
	static void serverThread(void* param);
};
