#pragma once

#include "mutex.hpp"

#include "LCD/ILI9341.hpp"
#include "LCD/FT6X36.hpp"

class App
{
public:
	using LCD = ILI9341<Color565>;
	using Callback_t = void(*)(App* nextApp);

	App(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback) : lcd{ lcd }, touch{ touch }, changeAppCallback{ changeAppCallback }, newAppCallback{ newAppCallback } {}
	virtual ~App() {}

	virtual void init() { deleteAble = false; running = true; }
	virtual void focusIn() {}
	// virtual void focusOut() {}
	virtual void deinit() { running = false; deleteAble = true; }

	virtual void draw() = 0;
	virtual void touchUpdate() {}
	virtual void back() {}

	bool isRunning() { return running; }
	bool isDeleteAble() { return deleteAble; }

	Mutex drawMutex;
	Mutex touchMutex;

protected:
	LCD& lcd;
	FT6X36& touch;

	bool running = false;
	bool deleteAble = true;

	Callback_t changeAppCallback = nullptr;
	Callback_t newAppCallback = nullptr;
};
