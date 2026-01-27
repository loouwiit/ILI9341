#include "colorInput.hpp"
#include <esp_log.h>
#include <cstring>

constexpr static char TAG[] = "AppColorInput";

void AppColorInput::init()
{
	content[0] = &bars;
	content[1] = &colorPreviewBoard;
	content[2] = &colorPreview;

	colorPreview.clickCallbackParam = this;
	colorPreview.releaseCallback = [](Finger& finger, void* param)
		{
			AppColorInput& self = *(AppColorInput*)param;
			self.back();
		};

	bars[0] = &bar[0];
	bars[1] = &bar[1];
	bars[2] = &bar[2];

	bar[0].position = BarFirstPosition + BarGap * 0;
	bar[1].position = BarFirstPosition + BarGap * 1;
	bar[2].position = BarFirstPosition + BarGap * 2;

	bar[0].barColor = BackgroundColor;
	bar[1].barColor = BackgroundColor;
	bar[2].barColor = BackgroundColor;

	bar[0].clickCallbackParam = this;
	bar[0].holdCallback = [](Finger& finger, void* param)
		{
			AppColorInput& self = *(AppColorInput*)param;
			self.color.R = self.bar[0].getValue();
			self.colorPreview.color = self.color;
			self.changeCallback(self.callbackParam);
		};
	bar[1].clickCallbackParam = this;
	bar[1].holdCallback = [](Finger& finger, void* param)
		{
			AppColorInput& self = *(AppColorInput*)param;
			self.color.G = self.bar[1].getValue();
			self.colorPreview.color = self.color;
			self.changeCallback(self.callbackParam);
		};
	bar[2].clickCallbackParam = this;
	bar[2].holdCallback = [](Finger& finger, void* param)
		{
			AppColorInput& self = *(AppColorInput*)param;
			self.color.B = self.bar[2].getValue();
			self.colorPreview.color = self.color;
			self.changeCallback(self.callbackParam);
		};
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
