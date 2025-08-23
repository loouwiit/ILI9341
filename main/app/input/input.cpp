#include "input.hpp"
#include <esp_log.h>
#include <cstring>

constexpr static char TAG[] = "AppInput";

void AppInput::init()
{
	keyBoard.start.y = 16 * InputSize + GapSize;
	keyBoard.end.x = LCD::ScreenSize.x;
	keyBoard.end.y = LCD::ScreenSize.y;

	for (unsigned char i = 0; i < KeyBoardLineSize;i++)
	{
		keys[0][i].position.x = BoardSize + i * (LCD::ScreenSize.x - BoardSize * 2) / 10;
		keys[1][i].position.x = BoardSize + i * (LCD::ScreenSize.x - BoardSize * 2) / 10 + 16 * KeySize / 2;
		keys[2][i].position.x = BoardSize + i * (LCD::ScreenSize.x - BoardSize * 2) / 10 + 16 * KeySize / 2;
		keys[3][i].position.x = BoardSize + i * (LCD::ScreenSize.x - BoardSize * 2) / 10;

		keys[0][i].position.y = GapSize + (16 * KeySize + GapSize) * 0;
		keys[1][i].position.y = GapSize + (16 * KeySize + GapSize) * 1;
		keys[2][i].position.y = GapSize + (16 * KeySize + GapSize) * 2;
		keys[3][i].position.y = GapSize + (16 * KeySize + GapSize) * 3;
	}
	keys[2][0].position.x -= 8 * KeySize;

	for (unsigned char i = 0; i < KeyBoardLineCount; i++)
	{
		for (unsigned char j = 0; j < KeyBoardLineSize; j++)
		{
			keyCallbackParam[i][j].self = this;
			keyCallbackParam[i][j].keyText = &keys[i][j];

			keys[i][j].scale = KeySize;
			keys[i][j].textColor = LCD::Color::White;
			keys[i][j].backgroundColor = BackgroundColor;
			keys[i][j].clickCallbackParam = &keyCallbackParam[i][j];
			keys[i][j].releaseCallback = keyBoardInput;
			keyBoardLine[i][j] = &keys[i][j];
		}
		keyBoard[i] = &keyBoardLine[i];
	}

	// shift
	keys[2][0].clickCallbackParam = this;
	keys[2][0].pressCallback = [](Finger&, void* param)
		{
			AppInput& self = *(AppInput*)param;
			self.shiftLongPressClock = clock() + ShiftLongPressClockThreshold;
		};
	keys[2][0].holdCallback = [](Finger&, void* param)
		{
			AppInput& self = *(AppInput*)param;
			using State = AppInput::KeyBoardState;
			State& state = self.keyBoardState;

			if (self.shiftLongPressClock < clock() && self.shiftLongPressClock != InfinityClock)
			{
				// long press
				self.shiftLongPressClock = InfinityClock; // 不会再次触发
				switch (state)
				{
				case State::Normal: state = State::Captial; break;
				case State::Shift: state = State::Captial; break;
				case State::Captial:state = State::Shift; break;
				default: state = State::Captial; break;
				}
				self.updateKeyBoardShift(self);
			}
		};
	keys[2][0].releaseCallback = [](Finger&, void* param)
		{
			AppInput& self = *(AppInput*)param;
			using State = AppInput::KeyBoardState;
			State& state = self.keyBoardState;

			if (clock() < self.shiftLongPressClock && self.shiftLongPressClock != InfinityClock)
			{
				// short press
				switch (state)
				{
				case State::Normal: state = State::Shift; break;
				case State::Shift: state = State::Normal; break;
				case State::Captial: state = State::Normal; break;
				default: state = State::Normal; break;
				}
				self.updateKeyBoardShift(self);
			}
		};

	// backspace
	keys[2][8].clickCallbackParam = this;
	keys[2][8].releaseCallback = [](Finger&, void* param)
		{
			AppInput& self = *(AppInput*)param;
			if (self.inputIndex == 0) return;
			self.inputIndex--;
			char old = self.inputBuffer[self.inputIndex];
			self.inputBuffer[self.inputIndex] = '\0';

			bool vailed = self.checker(self.inputBuffer);
			if (!vailed)
			{
				// roll back
				self.inputBuffer[self.inputIndex + 1] = '\0';
				self.inputBuffer[self.inputIndex] = old;
				self.inputIndex++;
			}
			self.focus();
		};

	keys[3][9].clickCallbackParam = this;
	keys[3][9].releaseCallback = [](Finger&, void* param)
		{
			AppInput& self = *(AppInput*)param;
			self.back();
		};

	updateKeyBoardShift(*this);

	for (unsigned char i = 0; i < KeyBoardLineCount; i++) for (unsigned char j = 0; j < KeyBoardLineSize; j++)
		keys[i][j].computeSize();
}

void AppInput::draw()
{
	lcd.clear();
	lcd.draw(inputText);
	lcd.draw(keyBoard);
}

void AppInput::touchUpdate()
{
	Finger finger[2] = { touch[0],touch[1] };

	if (finger[0].state == Finger::State::Press)
	{
		if (finger[0].position.y < keyBoard.start.y)
		{
			fingerActiveToMove[0] = true;
			lastFingerPosition[0] = finger[0].position;
			fingerMoveTotol[0] = {};
		}
		else fingerActiveToType[0] = true;
	}

	if (finger[1].state == Finger::State::Press)
	{
		if (finger[1].position.y < keyBoard.start.y)
		{
			fingerActiveToMove[1] = true;
			lastFingerPosition[1] = finger[1].position;
			fingerMoveTotol[1] = {};
		}
		else fingerActiveToType[0] = true;
	}

	if (finger[0].state != Finger::State::None && fingerActiveToType[0])
		keyBoard.finger(finger[0]);
	if (finger[1].state != Finger::State::None && fingerActiveToType[1])
		keyBoard.finger(finger[1]);

	if (finger[0].state == Finger::State::Realease)
	{
		if (fingerActiveToMove[0])
		{
			fingerActiveToMove[0] = false;
			releaseDetect();
		}
		else fingerActiveToType[0] = false;
	}

	if (finger[1].state == Finger::State::Realease)
	{
		if (fingerActiveToMove[1])
		{
			fingerActiveToMove[1] = false;
			releaseDetect();
		}
		else fingerActiveToType[1] = false;
	}

	if (fingerActiveToMove[0])
	{
		auto movement = finger[0].position - lastFingerPosition[0];
		fingerMoveTotol[0] += movement;
		offset += movement.x;
		lastFingerPosition[0] = finger[0].position;
	}
	if (fingerActiveToMove[1])
	{
		auto movement = finger[1].position - lastFingerPosition[1];
		fingerMoveTotol[1] += movement;
		offset += movement.x;
		lastFingerPosition[1] = finger[1].position;
	}
}

void AppInput::back()
{
	finishCallback(finishCallbackParam);
	changeAppCallback(nullptr);
}

void AppInput::setInputBuffer(char* inputBuffer)
{
	this->inputBuffer = inputBuffer;
	inputText.text = inputBuffer;
	inputIndex = strlen(inputBuffer);
}

void AppInput::updateKeyBoardShift(AppInput& self)
{
	auto& Keys = KeyBoardKey[(unsigned char)self.keyBoardState];

	for (unsigned char i = 0; i < KeyBoardLineCount; i++) for (unsigned char j = 0; j < KeyBoardLineSize; j++)
	{
		if (Keys[i][j] != nullptr)
			self.keys[i][j].text = Keys[i][j];
		else
			self.keys[i][j].text = "";
	}
}

void AppInput::keyBoardInput(Finger&, void* param)
{
	AppInput& self = *(((CallbackParam_t*)param)->self);
	LCD::Text& text = *(((CallbackParam_t*)param)->keyText);

	self.inputIndex++;
	self.inputBuffer[self.inputIndex] = '\0';
	self.inputBuffer[self.inputIndex - 1] = *text.text;
	bool vailed = self.checker(self.inputBuffer);
	if (!vailed)
	{
		self.inputIndex--;
		self.inputBuffer[self.inputIndex] = '\0';
	}
	self.focus();
}

void AppInput::releaseDetect()
{
	if (offset > 0)
		offset = 0;
}

void AppInput::focus()
{
	inputText.computeSize();
	auto RightPosition = inputText.endPosition.x;
	auto maxRightPosition = LCD::ScreenSize.x - FoucsRightSpareSize;
	if (RightPosition > maxRightPosition)
		offset = -inputText.getSize().x + maxRightPosition;

	auto maxLeftPosition = 0 + FoucsLeftSpareSize;
	if (RightPosition < maxLeftPosition)
	{
		auto targetPosition = -inputText.getSize().x + maxLeftPosition;
		if (targetPosition > 0)
			targetPosition = 0;
		offset = targetPosition;
	}
}
