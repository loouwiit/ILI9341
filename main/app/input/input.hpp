#pragma once

#include "app.hpp"
#include <ctime>

class AppInput final : public App
{
public:
	AppInput(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback) : App(lcd, touch, changeAppCallback, newAppCallback) {}
	AppInput(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback, char* inputBuffer) : App(lcd, touch, changeAppCallback, newAppCallback), inputBuffer{ inputBuffer } {}

	virtual void init() override final;
	// virtual void deinit() override final;

	virtual void draw() override final;
	virtual void touchUpdate() override final;
	virtual void back() override final;

	static bool defaultChecker(char*) { return true; }
	static void defaultCallback(char*) {}

	const char* description = "description";
	char* inputBuffer = nullptr;
	unsigned short inputIndex = 0;
	bool (*checker)(char*) = defaultChecker;
	void (*finishCallback)(char*) = defaultCallback;

private:
	constexpr static unsigned char InputSize = 2;
	constexpr static unsigned char KeySize = 3;
	constexpr static short GapSize = 1;
	constexpr static short BoardSize = 16;
	constexpr static short FoucsRightSpareSize = 16;
	constexpr static short FoucsLeftSpareSize = 64;
	constexpr static LCD::Color BackgroundColor = { 8,8,8 };

	LCD::Text inputText{ {0, 0}, inputBuffer, InputSize, LCD::Color::White, BackgroundColor };

	constexpr static clock_t InfinityClock = -1;
	constexpr static clock_t ShiftLongPressClockThreshold = 600;
	clock_t shiftLongPressClock = 0;
	enum class KeyBoardState : unsigned char
	{
		Normal = 0,
		Shift,
		Captial,
		Count,
	} keyBoardState = KeyBoardState::Normal;
	constexpr static unsigned char KeyBoardLineCount = 4;
	constexpr static unsigned char KeyBoardLineSize = 10;
	constexpr static const char* KeyBoardKey[(size_t)KeyBoardState::Count][KeyBoardLineCount][KeyBoardLineSize] = {
		{
		{"q","w","e","r","t","y","u","i","o","p"},
		{"a","s","d","f","g","h","j","k","l"},
		{"sh","z","x","c","v","b","n","m","<-"},
		{":","/","?","\""," ","(",")",",",".","|>"},
		}, {
		{"1","2","3","4","5","6","7","8","9","0"},
		{"!","@","#","$","%","^","&","*","~"},
		{"sh","<",">","[","]","{","}"," ","<-"},
		{";","\\","|","'"," ","-","+","_","=","|>"},
		}, {
		{"Q","W","E","R","T","Y","U","I","O","P"},
		{"A","S","D","F","G","H","J","K","L"},
		{"sh","Z","X","C","V","B","N","M","<-"},
		{":","/","?","\""," ","(",")",",",".","|>"},
		} };
	LCD::Layar<LayarClassicSize::Small>keyBoard{ KeyBoardLineCount };
	LCD::Layar<LayarClassicSize::Middle>keyBoardLine[KeyBoardLineCount]{};
	LCD::Text keys[KeyBoardLineCount][KeyBoardLineSize]{};
	struct CallbackParam_t { AppInput* self; LCD::Text* keyText; };
	CallbackParam_t keyCallbackParam[KeyBoardLineCount][KeyBoardLineSize]{};

	void static updateKeyBoardShift(AppInput& self);
	void static keyBoardInput(Finger&, void* param);

	constexpr static float moveThreshold2 = 100.0f;

	short& offset = inputText.position.x;
	bool fingerActiveToType[2] = { false, false };
	bool fingerActiveToMove[2] = { false, false };
	Vector2s lastFingerPosition[2]{};
	Vector2s fingerMoveTotol[2]{};

	void releaseDetect();
	void focus();
};
