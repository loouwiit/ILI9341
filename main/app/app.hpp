#pragma once

#include <mutex>

#include "frame.hpp"
#include "ILI9341.hpp"
#include "FT6X36.hpp"

using LCD = ILI9341<Color565>;

class App
{
public:
	using ExitCallback_t = void(*)(App* nextApp);

	App(LCD& lcd, FT6X36& touch, ExitCallback_t exitCallback) : lcd{ lcd }, touch{ touch }, exitCallback{ exitCallback } {}
	virtual ~App() {};

	virtual void init() {};
	virtual void deinit() {};

	virtual void draw() = 0;
	virtual void touchUpdate() = 0;

	std::mutex drawMutex;
	std::mutex touchMutex;

protected:
	LCD& lcd;
	FT6X36& touch;

	ExitCallback_t exitCallback = nullptr;
};
