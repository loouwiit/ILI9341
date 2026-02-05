#include "property.hpp"

#include <cstring>

#include "app/picture/picture.hpp"
#include "app/textEditor/textEditor.hpp"
#include "app/input/textInput.hpp"

#include "storge/fat.hpp"
#include "sys/stat.h"

void AppProperty::init()
{
	App::init();

	contents[0] = &title;
	contents[1] = &pathLayar;
	contents[2] = &typeText;
	contents[3] = &sizeDescripText;
	contents[4] = &sizeText;
	contents[5] = &deleteText;

	title.position.x = LCD::ScreenSize.x / 2;
	title.position.x -= title.computeSize().x / 2;
	title.computeSize();
	title.clickCallbackParam = this;
	title.releaseCallback = [](Finger&, void* param) { AppProperty& self = *(AppProperty*)param; self.back(); };

	pathLayar[0] = &pathDescripText;
	pathLayar[1] = &pathText;

	struct stat st {};
	if (stat(pathBuffer, &st) < 0)
	{
		// stat error
		typeText.text = AutoLnaguage{ "type:stat error", "类型:获取错误" };
		deleteText.text = "";
	}
	else
	{
		sizeText.number = st.st_size;
		if (S_ISREG(st.st_mode))
		{
			typeText.textColor = FileColor;
			typeText.text = AutoLnaguage{ "type:file", "类型:文件" };
		}
		else if (S_ISDIR(st.st_mode))
		{
			typeText.textColor = FloorColor;
			typeText.text = AutoLnaguage{ "type:dirent", "类型:目录" };
		}
		else
		{
			// st_mode error
			typeText.text = AutoLnaguage{ "type:unknow", "类型:未知" };
			deleteText.text = "";
		}
	}

	deleteText.computeSize();
	deleteText.clickCallbackParam = this;
	deleteText.releaseCallback = [](Finger&, void* param)
		{
			auto& self = *(AppProperty*)param;

			if (self.typeText.textColor == FileColor)
			{
				ESP_LOGI(TAG, "remove file %s", self.pathBuffer);
				if (!removeFile(self.pathBuffer))
					ESP_LOGW(TAG, "remove file failed");
			}
			else if (self.typeText.textColor == FloorColor)
			{
				ESP_LOGI(TAG, "remove floor %s", self.pathBuffer);
				if (!removeFloor(self.pathBuffer))
					ESP_LOGW(TAG, "remove floor failed");
			}

			self.back();
		};
}

void AppProperty::deinit()
{
	delete[] this->titleBuffer;
	this->titleBuffer = nullptr;
	App::deinit();
}

void AppProperty::draw()
{
	lcd.clear();
	lcd.draw(contents);
}

void AppProperty::touchUpdate()
{
	Finger finger[2] = { touch[0],touch[1] };

	if (finger[0].state == Finger::State::Press) do
	{
		fingerActive[0] = true;

		lastFingerPosition[0] = finger[0].position;
		fingerMoveTotol[0] = {};
		fingerActiveMovePath[0] = contents.isClicked(finger[0].position, pathLayar);
	} while (false);

	if (fingerActive[0]) do
	{
		auto movement = finger[0].position - lastFingerPosition[0];
		if (fingerActiveMovePath[0]) pathLayar.start.x += movement.x;
		else contents.start.y += movement.y;
		contents.start.y += movement.y;
		lastFingerPosition[0] = finger[0].position;
	} while (false);

	if (finger[0].state == Finger::State::Realease) do
	{
		if (abs2(fingerMoveTotol[0]) < moveThreshold2)
			click(finger[0]);
		fingerActive[0] = false;
		releaseDetect();
	} while (false);

	if (finger[1].state == Finger::State::Press) do
	{
		fingerActive[1] = true;

		lastFingerPosition[1] = finger[1].position;
		fingerMoveTotol[1] = {};
		fingerActiveMovePath[1] = contents.isClicked(finger[1].position, pathLayar);
	} while (false);

	if (fingerActive[1]) do
	{
		auto movement = finger[1].position - lastFingerPosition[1];
		fingerMoveTotol[1] += movement;
		if (fingerActiveMovePath[1]) pathLayar.start.x += movement.x;
		else contents.start.y += movement.y;
		lastFingerPosition[1] = finger[1].position;
	} while (false);

	if (finger[1].state == Finger::State::Realease) do
	{
		if (abs2(fingerMoveTotol[1]) < moveThreshold2)
			click(finger[1]);
		fingerActive[1] = false;
		releaseDetect();
	} while (false);
}

void AppProperty::back()
{
	changeAppCallback(nullptr);
}

void AppProperty::setTitle(const char* title)
{
	delete[] titleBuffer;

	auto* buffer = new char[strlen(title) + 1];
	strcpy(buffer, title);

	titleBuffer = buffer;

	this->title.text = titleBuffer;
	this->title.position.x = LCD::ScreenSize.x / 2;
	this->title.position.x -= this->title.computeSize().x / 2;
	this->title.computeSize();
}

void AppProperty::setTitleBuffer(const char* titleBuffer)
{
	delete[] this->titleBuffer;
	this->titleBuffer = nullptr;

	this->title.text = titleBuffer != nullptr ? titleBuffer : defaultTitle;
	this->title.position.x = LCD::ScreenSize.x / 2;
	this->title.position.x -= this->title.computeSize().x / 2;
	this->title.computeSize();
}

void AppProperty::setPath(const char* path)
{
	strcpy(pathBuffer, path);
	pathText.computeSize(); // 没有必要好像
}

void AppProperty::click(Finger finger)
{
	contents.finger(finger);
}

void AppProperty::releaseDetect()
{
	if (touch[0].state != Finger::State::Contact && touch[1].state != Finger::State::Contact)
	{
		if (contents.start.y > 0)
			contents.start.y = 0;
		if (pathLayar.start.x > ContentXOffset)
			pathLayar.start.x = ContentXOffset;
	}
}
