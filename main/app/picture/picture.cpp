#include "picture.hpp"

void AppPicture::init()
{
	App::init();

	path.text = fileName;
	path.position.y -= path.computeSize().y;

	pictureCountText.position = path.position;
	pictureCountText.position.x += path.computeSize().x + 8;

	file.read(&totol, mataDataSize);

	textTime = clock() + TextFadeTime;

	if (xTaskCreate([](void* param)
		{
			AppPicture& self = *(AppPicture*)param;
			while (self.running)
			{
				self.tryLoadNext();
				vTaskDelay(1);
			}
			self.deleteAble = true;
			vTaskDelete(nullptr);
		}, "picture", 4096, this, 2, nullptr) != pdTRUE)
	{
		deleteAble = true;
		nextFrameTime = (clock_t)-1;
		textTime = (clock_t)-1;
		path.text = "error:out of memory";
		pictureCountText.position.y = LCD::ScreenSize.y;
	}
}

void AppPicture::deinit()
{
	running = false;
}

void AppPicture::touchUpdate()
{
	if ((touch[0].state == Finger::State::Press && touch[1].state == Finger::State::Contact) ||
		(touch[1].state == Finger::State::Press && touch[0].state == Finger::State::Contact) ||
		(touch[0].state == Finger::State::Contact && touch[1].state == Finger::State::Contact))
	{
		changeAppCallback(nullptr);
	}
	else if (touch[0].state == Finger::State::Press ||
		touch[0].state == Finger::State::Contact ||
		touch[1].state == Finger::State::Press ||
		touch[1].state == Finger::State::Contact)
	{
		textTime = clock() + TextFadeTime;
		drawText();
	}
}

void AppPicture::back()
{
	changeAppCallback(nullptr);
}

void AppPicture::tryLoadNext()
{
	if (clock() < nextFrameTime) return;

	index = (index + 1) % totol;
	load(index);
}

void AppPicture::load(unsigned short index)
{
	LCD::Frame& frame = lcd;

	file.setOffset(mataDataSize + frameBufferSize * index);

	drawMutex.lock();

	// file.read(&frameTime, clockBufferSize);
	nextFrameTime = clock() + frameTime;

	file.read(&frame.buffer, frameBufferSize);

	drawText();

	drawMutex.unlock();
}

void AppPicture::drawText()
{
	if (clock() < textTime)
	{
		lcd.draw(path);
		lcd.draw(pictureCountText);
	}
}
