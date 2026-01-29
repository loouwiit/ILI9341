#include "colorInput.hpp"
#include <esp_log.h>
#include <cstring>

constexpr static char TAG[] = "AppColorInput";

void AppColorInput::init()
{
	content[0] = &colorPreviewBoard;
	content[1] = &colorPreview;
	content[2] = &bars;
	content[3] = &numbers;

	colorPreview.clickCallbackParam = this;
	colorPreview.releaseCallback = [](Finger& finger, void* param)
		{
			AppColorInput& self = *(AppColorInput*)param;
			self.back();
		};

	bars[0] = &bar[0];
	bars[1] = &bar[1];
	bars[2] = &bar[2];

	bar[0].barColor = BackgroundColor;
	bar[1].barColor = BackgroundColor;
	bar[2].barColor = BackgroundColor;

	bar[0].clickCallbackParam = this;
	bar[0].holdCallback = [](Finger& finger, void* param)
		{
			AppColorInput& self = *(AppColorInput*)param;
			self.color.R = self.number[0].number = self.bar[0].getValue();
			self.colorPreview.color = self.color;
			self.changeCallback(self.callbackParam);
		};
	bar[1].clickCallbackParam = this;
	bar[1].holdCallback = [](Finger& finger, void* param)
		{
			AppColorInput& self = *(AppColorInput*)param;
			self.color.G = self.number[1].number = self.bar[1].getValue();
			self.colorPreview.color = self.color;
			self.changeCallback(self.callbackParam);
		};
	bar[2].clickCallbackParam = this;
	bar[2].holdCallback = [](Finger& finger, void* param)
		{
			AppColorInput& self = *(AppColorInput*)param;
			self.color.B = self.number[2].number = self.bar[2].getValue();
			self.colorPreview.color = self.color;
			self.changeCallback(self.callbackParam);
		};

	numbers[0] = &number[0];
	numbers[1] = &number[1];
	numbers[2] = &number[2];

	number[0].position.y -= number[0].computeSize().y / 2;
	number[1].position.y -= number[1].computeSize().y / 2;
	number[2].position.y -= number[2].computeSize().y / 2;
}

void AppColorInput::draw()
{
	lcd.clear();
	lcd.draw(content);
}

void AppColorInput::touchUpdate()
{
	Finger finger[2] = { touch[0],touch[1] };

	if (finger[0].state != Finger::State::None)
		content.finger(finger[0]);
	if (finger[1].state != Finger::State::None)
		content.finger(finger[1]);
}

void AppColorInput::back()
{
	finishCallback(callbackParam);
	changeAppCallback(nullptr);
}
