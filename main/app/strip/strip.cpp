#include "app/strip/strip.hpp"

#include "../../strip.hpp"

#include "app/input/colorInput.hpp"

EXT_RAM_BSS_ATTR Strip strip{};

void AppStrip::init()
{
	App::init();

	title.position = { LCD::ScreenSize.x / 2, 0 };
	title.position.x -= title.computeSize().x / 2;
	title.computeSize();
	title.clickCallbackParam = this;
	title.releaseCallback = [](Finger&, void* param) { AppStrip& self = *(AppStrip*)param; self.changeAppCallback(nullptr); };

	contents.elementCount = 2;
	contents[0] = &title;
	contents[1] = &stripText;

	updateState();

	stripText.clickCallbackParam = this;
	stripText.releaseCallback = [](Finger&, void* param)
		{
			AppStrip& self = *(AppStrip*)param;
			if (strip.empty())
			{
				// 启动strip
				strip = Strip{ {gpioNum, GPIO::Mode::GPIO_MODE_OUTPUT}, ledCount, led_model_t::LED_MODEL_WS2812 };

				// 输入颜色
				auto* appColorInput = new AppColorInput{ self.lcd,self.touch,self.changeAppCallback,self.newAppCallback };
				self.appColorInput = appColorInput;

				appColorInput->callbackParam = &self;
				appColorInput->changeCallback = [](void* param)
					{
						AppStrip& self = *(AppStrip*)param;

						for (int i = 0; i < self.ledCount; i++)
							strip[i] = ((AppColorInput*)self.appColorInput)->getColor();
						strip.flush();
					};

				self.newAppCallback(appColorInput);
			}
			else
			{
				strip.clear();
				strip = Strip{};
			}
			self.updateState();
		};
}

void AppStrip::draw()
{
	lcd.clear();
	lcd.draw(contents);
}

void AppStrip::touchUpdate()
{
	Finger finger[2] = { touch[0],touch[1] };

	if (finger[0].state == Finger::State::Press)
	{
		fingerActive[0] = true;
		lastFingerPosition[0] = finger[0].position;
		fingerMoveTotol[0] = {};
	}
	else if (finger[0].state == Finger::State::Realease)
	{
		if (abs2(fingerMoveTotol[0]) < moveThreshold2)
			click(finger[0]);
		fingerActive[0] = false;
		releaseDetect();
	}

	if (finger[1].state == Finger::State::Press)
	{
		fingerActive[1] = true;
		lastFingerPosition[1] = finger[1].position;
		fingerMoveTotol[1] = {};
	}
	else if (finger[1].state == Finger::State::Realease)
	{
		if (abs2(fingerMoveTotol[1]) < moveThreshold2)
			click(finger[1]);
		fingerActive[1] = false;
		releaseDetect();
	}

	if (fingerActive[0])
	{
		auto movement = finger[0].position - lastFingerPosition[0];
		fingerMoveTotol[0] += movement;
		offset += movement.y;
		lastFingerPosition[0] = finger[0].position;
	}
	if (fingerActive[1])
	{
		auto movement = finger[1].position - lastFingerPosition[1];
		fingerMoveTotol[1] += movement;
		offset += movement.y;
		lastFingerPosition[1] = finger[1].position;
	}
}

void AppStrip::back()
{
	changeAppCallback(nullptr);
}

void AppStrip::updateState()
{
	stripText.text = strip.empty() ? AutoLnaguage{ "strip:off","灯带：关" } : AutoLnaguage{ "strip:on","灯带：开" };
	stripText.computeSize();
}

void AppStrip::click(Finger finger)
{
	contents.finger(finger);
}

void AppStrip::releaseDetect()
{
	if (touch[0].state != Finger::State::Contact && touch[1].state != Finger::State::Contact)
	{
		if (offset > 0)
			offset = 0;
	}
}
