#pragma once

#include "app.hpp"

class AppTouch final : public App
{
public:
	AppTouch(LCD& lcd, FT6X36& touch, ExitCallback_t exitCallback) : App(lcd, touch, exitCallback) {}

	virtual void init() override;

	virtual void draw() override;
	virtual void touchUpdate() override;
	virtual void back() override;

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
