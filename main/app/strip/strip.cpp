#include "app/strip/strip.hpp"
#include "app/input/colorInput.hpp"

#include "task.hpp"
#include <cstring>
#include "strip/manager.hpp"

EXT_RAM_BSS_ATTR Strip strip{};
EXT_RAM_BSS_ATTR Strip::Manager stripManager{ strip };

void AppStrip::init()
{
	App::init();

	stripManager.stop();

	title.position = { LCD::ScreenSize.x / 2, 0 };
	title.position.x -= title.computeSize().x / 2;
	title.computeSize();
	title.clickCallbackParam = this;
	title.releaseCallback = [](Finger&, void* param) { AppStrip& self = *(AppStrip*)param; self.changeAppCallback(nullptr); };

	contents[0] = &title;
	contents[1] = &stripText;
	contents[2] = &stepLayar;
	contents[3] = &lastTimeLayar;
	contents[4] = &lastTimeBar;
	contents[5] = &ledLayar;

	updateState();

	stripText.clickCallbackParam = this;
	stripText.releaseCallback = [](Finger&, void* param)
		{
			AppStrip& self = *(AppStrip*)param;
			if (strip.empty())
			{
				// 启动strip驱动
				strip = Strip{ {GpioNum, GPIO::Mode::GPIO_MODE_OUTPUT}, LedCount, led_model_t::LED_MODEL_WS2812 };
				strip.clear();
				stripManager.init();

				if (testFile(ConfigFilePath))
				{
					IFile file{};
					file.open(ConfigFilePath);
					if (file.isOpen())
					{
						stripManager.load(file);
						stripManager.apply();
					}
				}
			}
			else
			{
				if (self.fileNeedWrite)
				{
					OFile file{};
					file.open(ConfigFilePath);
					if (file.isOpen() && stripManager.save(file) != 0)
						self.fileNeedWrite = false;
				}

				stripManager.deinit();
				strip.clear();
				strip = Strip{};
			}
			self.updateState();
		};

	stepLayar[0] = &stepText;
	stepLayar[1] = &stepLeft;
	stepLayar[2] = &stepRight;
	stepLayar[3] = &stepAdd;
	stepLayar[4] = &stepRemove;

	stepText.text = stepTextBuffer;

	stepLeft.computeSize();
	stepLeft.clickCallbackParam = this;
	stepLeft.releaseCallback = [](Finger&, void* param)
		{
			stripManager.last();
			stripManager.apply();

			auto& self = *(AppStrip*)param;

			for (uint32_t i = 0; i < LedCount; i++)
				self.leds[i].color = stripManager[i];

			sprintf(self.stepTextBuffer, AutoLnaguage{ "step:%d", "步骤:%d" }, stripManager.getId());
			self.lastTimeBar.setValue(stripManager.getLastTime() / 5);
			sprintf(self.lastTimeTextBuffer, AutoLnaguage{ "time:%dms", "时长:%dms" }, stripManager.getLastTime());
		};

	stepRight.computeSize();
	stepRight.clickCallbackParam = this;
	stepRight.releaseCallback = [](Finger&, void* param)
		{
			stripManager.next();
			stripManager.apply();

			auto& self = *(AppStrip*)param;

			for (uint32_t i = 0; i < LedCount; i++)
				self.leds[i].color = stripManager[i];

			sprintf(self.stepTextBuffer, AutoLnaguage{ "step:%d", "步骤:%d" }, stripManager.getId());
			self.lastTimeBar.setValue(stripManager.getLastTime() / 5);
			sprintf(self.lastTimeTextBuffer, AutoLnaguage{ "time:%dms", "时长:%dms" }, stripManager.getLastTime());
		};

	stepAdd.computeSize();
	stepAdd.clickCallbackParam = this;
	stepAdd.releaseCallback = [](Finger&, void* param)
		{
			stripManager.add();
			stripManager.apply();

			auto& self = *(AppStrip*)param;

			for (uint32_t i = 0; i < LedCount; i++)
				self.leds[i].color = stripManager[i];

			sprintf(self.stepTextBuffer, AutoLnaguage{ "step:%d", "步骤:%d" }, stripManager.getId());
			self.lastTimeBar.setValue(stripManager.getLastTime() / 5);
			sprintf(self.lastTimeTextBuffer, AutoLnaguage{ "time:%dms", "时长:%dms" }, stripManager.getLastTime());
		};

	stepRemove.computeSize();
	stepRemove.clickCallbackParam = this;
	stepRemove.releaseCallback = [](Finger&, void* param)
		{
			stripManager.remove();
			stripManager.apply();

			auto& self = *(AppStrip*)param;

			for (uint32_t i = 0; i < LedCount; i++)
				self.leds[i].color = stripManager[i];

			sprintf(self.stepTextBuffer, AutoLnaguage{ "step:%d", "步骤:%d" }, stripManager.getId());
			self.lastTimeBar.setValue(stripManager.getLastTime() / 5);
			sprintf(self.lastTimeTextBuffer, AutoLnaguage{ "time:%dms", "时长:%dms" }, stripManager.getLastTime());
		};

	lastTimeLayar[0] = &lastTimeText;
	lastTimeLayar[1] = &lastTimeAdd;
	lastTimeLayar[2] = &lastTimeSub;

	lastTimeAdd.computeSize();
	lastTimeAdd.clickCallbackParam = this;
	lastTimeAdd.releaseCallback = [](Finger&, void* param)
		{
			auto time = stripManager.getLastTime();
			time += LastTimeStep;
			if (time > 1250)
				time = 1250;
			stripManager.setLastTime(time);

			auto& self = *(AppStrip*)param;
			self.lastTimeBar.setValue(time / 5);
			sprintf(self.lastTimeTextBuffer, AutoLnaguage{ "time:%dms", "时长:%dms" }, time);
		};

	lastTimeSub.computeSize();
	lastTimeSub.clickCallbackParam = this;
	lastTimeSub.releaseCallback = [](Finger&, void* param)
		{
			auto time = stripManager.getLastTime();
			if (time > LastTimeStep)
				time -= LastTimeStep;
			else time = 0;
			stripManager.setLastTime(time);

			auto& self = *(AppStrip*)param;
			self.lastTimeBar.setValue(time / 5);
			sprintf(self.lastTimeTextBuffer, AutoLnaguage{ "time:%dms", "时长:%dms" }, time);
		};

	lastTimeText.text = lastTimeTextBuffer;
	lastTimeBar.clickCallbackParam = this;
	lastTimeBar.holdCallback = [](Finger&, void* param)
		{
			auto& self = *(AppStrip*)param;

			stripManager.setLastTime(self.lastTimeBar.getValue() / 10 * 50);
			sprintf(self.lastTimeTextBuffer, AutoLnaguage{ "time:%dms", "时长:%dms" }, stripManager.getLastTime());
		};

	for (uint32_t i = 0; i < LedCount; i++)
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
				appColorInput->setColor(stripManager[index]);
				appColorInput->callbackParam = param;
				appColorInput->changeCallback = [](void* param)
					{
						auto& self = **(AppStrip**)param;
						int index = (AppStrip**)param - self.ledParams;
						strip[index] = stripManager[index] = ((AppColorInput*)self.appColorInput)->getColor();
						strip.flush();
					};
				appColorInput->finishCallback = [](void* param)
					{
						auto& self = **(AppStrip**)param;
						int index = (AppStrip**)param - self.ledParams;
						strip[index] = stripManager[index] = ((AppColorInput*)self.appColorInput)->getColor();
						strip.flush();
						self.leds[index].color = ((AppColorInput*)self.appColorInput)->getColor();
					};

				self.newAppCallback(appColorInput);
			};
	}
}

void AppStrip::deinit()
{
	// background service
	if (!strip.empty())
		stripManager.start();
	App::deinit();
}

void AppStrip::draw()
{
	lcd.clear();
	lcd.draw(contents);
}

void AppStrip::touchUpdate()
{
	Finger finger[2] = { touch[0],touch[1] };

	if (finger[0].state == Finger::State::Press) do
	{
		fingerActive[0] = true;

		if ((fingerClickTime[0] = contents.isClicked(finger[0].position, lastTimeBar))) break;

		lastFingerPosition[0] = finger[0].position;
		fingerMoveTotol[0] = {};
		fingerMoveLeds[0] = contents.isClicked(finger[0].position, ledLayar);
	} while (false);

	if (fingerActive[0]) do
	{
		if (fingerClickTime[0])
		{
			contents.finger(finger[0], lastTimeBar);
			break;
		}

		auto movement = finger[0].position - lastFingerPosition[0];
		fingerMoveTotol[0] += movement;
		if (fingerMoveLeds[0]) ledLayar.start.x += movement.x;
		else contents.start.y += movement.y;
		lastFingerPosition[0] = finger[0].position;
	} while (false);

	if (finger[0].state == Finger::State::Realease) do
	{
		if (fingerClickTime[0]) break;
		if (abs2(fingerMoveTotol[0]) < moveThreshold2)
			click(finger[0]);
		fingerActive[0] = false;
		releaseDetect();
	} while (false);

	if (finger[1].state == Finger::State::Press) do
	{
		fingerActive[1] = true;

		if ((fingerClickTime[1] = contents.isClicked(finger[1].position, lastTimeBar))) break;

		lastFingerPosition[1] = finger[1].position;
		fingerMoveTotol[1] = {};
		fingerMoveLeds[1] = contents.isClicked(finger[1].position, ledLayar);
	} while (false);

	if (fingerActive[1]) do
	{
		if (fingerClickTime[1])
		{
			contents.finger(finger[1], lastTimeBar);
			break;
		}

		auto movement = finger[1].position - lastFingerPosition[1];
		fingerMoveTotol[1] += movement;
		if (fingerMoveLeds[1]) ledLayar.start.x += movement.x;
		else contents.start.y += movement.y;
		lastFingerPosition[1] = finger[1].position;
	} while (false);

	if (finger[1].state == Finger::State::Realease) do
	{
		if (fingerClickTime[1]) break;
		if (abs2(fingerMoveTotol[1]) < moveThreshold2)
			click(finger[1]);
		fingerActive[1] = false;
		releaseDetect();
	} while (false);
}

void AppStrip::back()
{
	changeAppCallback(nullptr);
}

void AppStrip::updateState()
{
	if (strip.empty())
	{
		stripText.text = AutoLnaguage{ "strip:off","灯带:关" };
		contents.elementCount = 2;
	}
	else
	{
		stripText.text = AutoLnaguage{ "strip:on","灯带:开" };
		contents.elementCount = ContensSize;

		for (uint32_t i = 0; i < LedCount; i++)
			leds[i].color = stripManager[i] = strip[i];

		sprintf(stepTextBuffer, AutoLnaguage{ "step:%d", "步骤:%d" }, stripManager.getId());
		lastTimeBar.setValue(stripManager.getLastTime() / 5);
		sprintf(lastTimeTextBuffer, AutoLnaguage{ "time:%dms", "时长:%dms" }, stripManager.getLastTime());
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
