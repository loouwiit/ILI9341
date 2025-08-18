#pragma once

#include "finger.hpp"

class Clickable
{
public:
	using Function = void(*)(Finger& finger, void* param);
	static void emptyCallback(Finger&, void*) {};

	Function pressCallback = emptyCallback;
	Function holdCallback = emptyCallback;
	Function releaseCallback = emptyCallback;

	void* clickCallbackParam = nullptr;

	virtual bool isClicked(Vector2s point) = 0;
	virtual void finger(Finger finger);
};
