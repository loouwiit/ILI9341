#pragma once

#include <mutex>

#include "frame.hpp"
#include "ILI9341.hpp"
#include "FT6X36.hpp"

class App
{
public:
	using LCD = ILI9341<Color565>;
	using ExitCallback_t = void(*)(App* nextApp);

	App(LCD& lcd, FT6X36& touch, ExitCallback_t exitCallback) : lcd{ lcd }, touch{ touch }, exitCallback{ exitCallback } {}
	virtual ~App() {};

	virtual void init() { deleteAble = false; running = true; };
	virtual void deinit() { running = false; deleteAble = true; };

	virtual void draw() = 0;
	virtual void touchUpdate() {};
	virtual void back() {};

	bool isRunning() { return running; }
	bool isDeleteAble() { return deleteAble; }

	std::mutex drawMutex;
	std::mutex touchMutex;

protected:
	LCD& lcd;
	FT6X36& touch;

	bool running = false;
	bool deleteAble = true;

	ExitCallback_t exitCallback = nullptr;
};
