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

	contents[0] = &title;
	contents[1] = &stripText;
	contents[2] = &ledLayar;

	updateState();

	stripText.clickCallbackParam = this;
	stripText.releaseCallback = [](Finger&, void* param)
		{
			AppStrip& self = *(AppStrip*)param;
			if (strip.empty())
			{
				// 启动strip
				strip = Strip{ {gpioNum, GPIO::Mode::GPIO_MODE_OUTPUT}, ledCount, led_model_t::LED_MODEL_WS2812 };
				strip.clear();
				self.contents.elementCount = 3;
			}
			else
			{
				strip.clear();
				strip = Strip{};
			}
			self.updateState();
		};

	ledLayar.start.y = 16 * TitleSize + GapSize + (16 * TextSize + GapSize) * 1;
	ledLayar.end.y = ledLayar.start.y + 16 * TextSize;
	for (uint32_t i = 0; i < ledCount; i++)
	{
		ledLayar[2 * i] = &ledBoards[i];
		ledLayar[2 * i + 1] = &leds[i];

		if (i == 0) ledBoards[i].start.x = ContentXOffset;
		else ledBoards[i].start.x = ledBoards[i - 1].end.x + GapSize;
		ledBoards[i].end = ledBoards[i].start + Vector2s{ TextSize, TextSize } * 16;
		ledBoards[i].color = BackgroundColor;

		leds[i].start = ledBoards[i].start + Vector2s{ BoardSize, BoardSize };
		leds[i].end = ledBoards[i].end - Vector2s{ BoardSize, BoardSize };

		ledParams[i] = this;
		ledBoards[i].clickCallbackParam = &ledParams[i];
		ledBoards[i].releaseCallback = [](Finger& finger, void* param)
			{
				// 输入颜色
				auto& self = **(AppStrip**)param;
				int index = (AppStrip**)param - self.ledParams;
				auto* appColorInput = new AppColorInput{ self.lcd,self.touch,self.changeAppCallback,self.newAppCallback };
				self.appColorInput = appColorInput;
				appColorInput->setColor(self.leds[index].color);
				appColorInput->callbackParam = param;
				appColorInput->changeCallback = [](void* param)
					{
						auto& self = **(AppStrip**)param;
						int index = (AppStrip**)param - self.ledParams;
						strip[index] = ((AppColorInput*)self.appColorInput)->getColor();
						strip.flush();
					};
				appColorInput->finishCallback = [](void* param)
					{
						auto& self = **(AppStrip**)param;
						int index = (AppStrip**)param - self.ledParams;
						strip[index] = ((AppColorInput*)self.appColorInput)->getColor();
						self.leds[index].color = ((AppColorInput*)self.appColorInput)->getColor();
					};

				self.newAppCallback(appColorInput);
			};
	}
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
		fingerMoveLeds[0] = ledLayar.start.y < finger[0].position.y && finger[0].position.y < ledLayar.end.y;
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
		fingerMoveLeds[1] = ledLayar.start.y < finger[1].position.y && finger[1].position.y < ledLayar.end.y;
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
		if (fingerMoveLeds[0]) ledLayar.start.x += movement.x;
		else contents.start.y += movement.y;
		lastFingerPosition[0] = finger[0].position;
	}
	if (fingerActive[1])
	{
		auto movement = finger[1].position - lastFingerPosition[1];
		fingerMoveTotol[1] += movement;
		if (fingerMoveLeds[1]) ledLayar.start.x += movement.x;
		else contents.start.y += movement.y;
		lastFingerPosition[1] = finger[1].position;
	}
}

void AppStrip::back()
{
	changeAppCallback(nullptr);
}

void AppStrip::updateState()
{
	if (strip.empty())
	{
		stripText.text = AutoLnaguage{ "strip:off","灯带：关" };
		contents.elementCount = 2;
	}
	else
	{
		stripText.text = AutoLnaguage{ "strip:on","灯带：开" };
		contents.elementCount = 3;

		for (uint32_t i = 0; i < ledCount; i++)
			leds[i].color = (LCD::Color)strip[i];
	}
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
		if (contents.start.y > 0)
			contents.start.y = 0;
		if (ledLayar.start.x > 0)
			ledLayar.start.x = 0;
	}
}
