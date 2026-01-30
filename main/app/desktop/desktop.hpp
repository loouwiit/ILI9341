#pragma once

#include "app/app.hpp"
#include "LCD/autoLanguage.hpp"

class AppDesktop final : public App
{
public:
	AppDesktop(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback) : App(lcd, touch, changeAppCallback, newAppCallback) {}

	virtual void init() override final;

	virtual void draw() override final;
	virtual void touchUpdate() override final;
	virtual void back() override final;

private:
	constexpr static Vector2s StaticOffset = { 30,50 };
	constexpr static unsigned short GapSize = 40;
	constexpr static unsigned short BlockSize = 100;
	constexpr static unsigned short TextGapSize = 2;
	constexpr static unsigned char TextSize = 2;

	constexpr static float moveThreshold2 = 100.0f;

	constexpr static unsigned char ApplicationSize = 7;

	const AutoLnaguage ApplicationName[ApplicationSize] = { {"setting","设置"}, {"clock","时钟"}, {"tracker","接收器"}, {"server","服务器"}, {"explorer","文件\n浏览"},{"tetris", "俄罗斯方块"}, {"strip", "灯带"} };

	LCD::Layar<LayarClassicSize::Large> applications{ ApplicationSize * 2 };
	LCD::Rectangle applicationRectangle[ApplicationSize]{};
	LCD::Text applicationText[ApplicationSize]{};

	short& offset = applications.start.x;
	bool fingerActive[2] = { false, false };
	Vector2s lastFingerPosition[2]{};
	Vector2s fingerMoveTotol[2]{};

	App* appFactory(unsigned char index);

	void click(Finger finger);
};
