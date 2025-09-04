#include "picture.hpp"

void AppPicture::init()
{
	App::init();

	path.text = fileName;
	path.position.y -= path.computeSize().y;

	pictureCountText.position = path.position;
	pictureCountText.position.x += path.computeSize().x + 8;

	file.read(&totol, sizeof(totol));
	file.read(&scale, sizeof(scale));

	if (scale != 1 && scale != 2 && scale != 4 && scale != 5 && scale != 8) scale = 1;

	frameSize = LCD::ScreenSize / scale;
	frameBufferSize = sizeof(LCD::Color) * frameSize.y * frameSize.x;

	textTime = clock() + TextFadeTime;
}

void AppPicture::draw()
{
	tryLoadNext();
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
	LCD::Frame& frame = lcd.getFrame();

	file.setOffset(mataDataSize + frameBufferSize * index);

	// file.read(&frameTime, clockBufferSize);
	nextFrameTime = clock() + frameTime;

	file.read(&frame.buffer, frameBufferSize);
	scaleBuffer();

	drawText();
}

void AppPicture::scaleBuffer()
{
	if (scale <= 1) return;

	LCD::Frame& frame = lcd.getFrame();
	LCD::Color* buffer = &frame.buffer[0][0];
	LCD::Color color;

	Vector2s now{};
	Vector2s offset{};
	int bufferIndex = frameSize.y * frameSize.x - 1;

	for (now.y = (LCD::ScreenSize.y / scale) - 1; now.y >= 0; now.y--) for (now.x = (LCD::ScreenSize.x / scale) - 1; now.x >= 0; now.x--)
	{
		color = buffer[bufferIndex--];
		for (offset.y = 0; offset.y < scale; offset.y++) for (offset.x = 0; offset.x < scale; offset.x++)
			frame[now * scale + offset] = color;
	}
}

void AppPicture::drawText()
{
	if (clock() < textTime)
	{
		lcd.draw(path);
		lcd.draw(pictureCountText);
	}
}
