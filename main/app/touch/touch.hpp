#pragma once

#include "app.hpp"

class AppTouch final : public App
{
public:
	AppTouch(LCD& lcd, FT6X36& touch, ExitCallback_t exitCallback) : App(lcd, touch, exitCallback) {}

	virtual void init() override;

	virtual void draw() override;
	virtual void touchUpdate() override;

private:
	LCD::Number<unsigned> interruptCount{ {250,100}, 0 };

	LCD::Number<unsigned> number{ {10,10} };

	LCD::Number<unsigned> state[2]{
		{ {10,10 + 16 * 1},(unsigned)FT6X36::Finger::State::None },
		{ {10,10 + 16 * 2},(unsigned)FT6X36::Finger::State::None }
	};

	LCD::Layar<2> line1Clear{};
	LCD::Rectangle line1XClear{ {0,0},{320,1}, LCD::Color::Black };
	LCD::Rectangle line1YClear{ {0,0},{1,240}, LCD::Color::Black };

	LCD::Layar<2> line1{};
	LCD::Rectangle line1X{ {0,0},{320,1}, LCD::Color::Red };
	LCD::Rectangle line1Y{ {0,0},{1,240}, LCD::Color::Red };

	LCD::Layar<2> line2Clear{};
	LCD::Rectangle line2XClear{ {0,0},{320,1}, LCD::Color::Black };
	LCD::Rectangle line2YClear{ {0,0},{1,240}, LCD::Color::Black };

	LCD::Layar<2> line2{};
	LCD::Rectangle line2X{ {0,0},{320,1}, LCD::Color::Blue };
	LCD::Rectangle line2Y{ {0,0},{1,240}, LCD::Color::Blue };
};
