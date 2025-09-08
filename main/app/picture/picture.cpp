#include "picture.hpp"

#include "yuv.hpp"

void AppPicture::init()
{
	App::init();

	path.text = fileName;
	path.position.y -= path.computeSize().y;

	pictureCountText.position = path.position;
	pictureCountText.position.x += path.computeSize().x + 8;

	file.read(&totol, sizeof(totol));
	file.read(&scale, sizeof(scale));

	yuv420Mode = scale & (1 << 7);
	scale &= ~(1 << 7);

	if (scale != 1 && scale != 2 && scale != 4 && scale != 5 && scale != 8) scale = 1;

	frameReadSize = LCD::ScreenSize / scale;
	if (yuv420Mode)
	{
		frameReadSize = frameReadSize / 2;
		frameReadLineSize = (sizeof(LCD::Color) + sizeof(uint16_t)) * frameReadSize.x;
	}
	else frameReadLineSize = sizeof(LCD::Color) * frameReadSize.x;
	frameBufferSize = frameReadLineSize * frameReadSize.y;

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

	for (short i = 0; i < frameReadSize.y; i++)
		file.read(&frame.buffer[i], frameReadLineSize);

	if (yuv420Mode) yuv420Decode();
	if (scale > 1) scaleBuffer();

	drawText();
}

void AppPicture::yuv420Decode()
{
	LCD::Frame& frame = lcd.getFrame();
	uint16_t buffer = 0;
	YUV yuv[4]{};

	LCD::Color colorDebug;

	Vector2s now{};

	for (now.y = frameReadSize.y - 1; now.y >= 0; now.y--) for (now.x = frameReadSize.x - 1; now.x >= 0; now.x--)
	{
		buffer = (uint16_t)frame[now.y][now.x * 2 + 0];
		yuv[0] = yuv[1] = yuv[2] = yuv[3] = {
			(uint8_t)(((buffer >> 10) & 0x1F) << 3),
			(int8_t)((((buffer >> 5) & 0x1F) << 3) - (0x7F & 0xF8)),
			(int8_t)((((buffer >> 0) & 0x1F) << 3) - (0x7F & 0xF8)) };

		if (buffer >> 15) for (int i = 0; i < 4; i++) yuv[i].y += 0x07;

		buffer = (uint16_t)frame[now.y][now.x * 2 + 1];
		yuv[1].y += ((buffer >> 10) & 0x1F) << 3;
		yuv[2].y += ((buffer >> 5) & 0x1F) << 3;
		yuv[3].y += ((buffer >> 0) & 0x1F) << 3;

		colorDebug = (LCD::Color)yuv[0]; 
		colorDebug = (LCD::Color)yuv[1];
		colorDebug = (LCD::Color)yuv[2]; // 247 -32 0 -> G = 64
		colorDebug = (LCD::Color)yuv[3];

		frame[now * 2 + Vector2s{ 0, 0 }] = (LCD::Color)yuv[0];
		frame[now * 2 + Vector2s{ 0, 1 }] = (LCD::Color)yuv[1];
		frame[now * 2 + Vector2s{ 1, 0 }] = (LCD::Color)yuv[2];
		frame[now * 2 + Vector2s{ 1, 1 }] = (LCD::Color)yuv[3];
	}
}

void AppPicture::scaleBuffer()
{
	LCD::Frame& frame = lcd.getFrame();
	LCD::Color color;

	Vector2s now{};
	Vector2s offset{};

	for (now.y = (LCD::ScreenSize.y / scale) - 1; now.y >= 0; now.y--) for (now.x = (LCD::ScreenSize.x / scale) - 1; now.x >= 0; now.x--)
	{
		color = frame[now];
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
