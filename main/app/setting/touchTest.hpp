#pragma once

#include "app.hpp"

class TouchTest final : public App
{
public:
	TouchTest(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback) : App(lcd, touch, changeAppCallback, newAppCallback) {}

	virtual void init() override final;

	virtual void draw() override final;
	virtual void touchUpdate() override final;
	virtual void back() override final;

private:
	LCD::Layar<LayarClassicSize::Small> count{ 4 };
	LCD::Number<unsigned> interruptCount{ {10,10} };
	LCD::Number<unsigned> eventCount[3]{ {{10,230 - 16 * 3}}, {{10,230 - 16 * 2}}, {{10,230 - 16 * 1}} };

	LCD::Number<unsigned> state[2]{
		{ {10,10 + 16 * 1},(unsigned)Finger::State::None },
		{ {10,10 + 16 * 2},(unsigned)Finger::State::None }
	};

	LCD::Layar<LayarClassicSize::Pair> line1Clear{ 2 };
	LCD::Rectangle line1XClear{ {0,0},{320,1}, LCD::Color::Black };
	LCD::Rectangle line1YClear{ {0,0},{1,240}, LCD::Color::Black };

	LCD::Layar<LayarClassicSize::Pair> line1{ 2 };
	LCD::Rectangle line1X{ {0,0},{320,1}, LCD::Color::Red };
	LCD::Rectangle line1Y{ {0,0},{1,240}, LCD::Color::Red };

	LCD::Layar<LayarClassicSize::Pair> line2Clear{ 2 };
	LCD::Rectangle line2XClear{ {0,0},{320,1}, LCD::Color::Black };
	LCD::Rectangle line2YClear{ {0,0},{1,240}, LCD::Color::Black };

	LCD::Layar<LayarClassicSize::Pair> line2{ 2 };
	LCD::Rectangle line2X{ {0,0},{320,1}, LCD::Color::Blue };
	LCD::Rectangle line2Y{ {0,0},{1,240}, LCD::Color::Blue };
};
