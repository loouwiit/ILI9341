#include "textEditor.hpp"

#include <cstring>

void AppTextEditor::init()
{
	App::init();

	contents[0] = &title;
	contents[1] = &fileLayar;

	title.text = fileName;
	title.position.x -= title.computeSize().x / 2;
	if (title.position.x < 0) title.position.x = 0;
	title.computeSize();
	title.clickCallbackParam = this;
	title.releaseCallback = [](Finger&, void* param) { AppTextEditor& self = *(AppTextEditor*)param; self.changeAppCallback(nullptr); };

	fileLayar.start.x = ContentXOffset;
	fileLayar.start.y = 16 * TitleSize + GapSize;

	for (unsigned char i = 0; i < LineCount; i++)
	{
		clickCallbackParam[i].self = this;
		clickCallbackParam[i].index = i;

		lines[i].position.y = (16 * TextSize + GapSize) * i;
		lines[i].textColor = LCD::Color::White;
		lines[i].backgroundColor = BackgroundColor;
		lines[i].scale = TextSize;
		lines[i].text = lineBuffers[i] = new char[LineSize];
		lineBuffers[i][0] = '\0';
		lines[i].clickCallbackParam = &clickCallbackParam[i];
		lines[i].releaseCallback = [](Finger&, void* param)
			{
				auto& self = *((ClickCallbackParam_t*)param)->self;
				auto index = ((ClickCallbackParam_t*)param)->index;
				self.clickCallback(index);
			};

		fileLayar[i] = &lines[i];
	}

	fileOffset = 0;
	fileOffset = loadText(fileOffset);
}

void AppTextEditor::deinit()
{
	running = false;
	for (unsigned char i = 0; i < LineCount; i++)
	{
		delete[] lineBuffers[i];
		lines[i].text = lineBuffers[i] = nullptr;
	}
	deleteAble = true;
}

void AppTextEditor::draw()
{
	lcd.clear();
	lcd.draw(contents);
}

void AppTextEditor::touchUpdate()
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
		releaseDetect();
		fingerActive[0] = false;
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
		releaseDetect();
		fingerActive[1] = false;
	}

	if (fingerActive[0])
	{
		auto movement = finger[0].position - lastFingerPosition[0];
		fingerMoveTotol[0] += movement;
		offset += movement;
		lastFingerPosition[0] = finger[0].position;
	}
	if (fingerActive[1])
	{
		auto movement = finger[1].position - lastFingerPosition[1];
		fingerMoveTotol[1] += movement;
		offset += movement;
		lastFingerPosition[1] = finger[1].position;
	}
}

void AppTextEditor::back()
{
	changeAppCallback(nullptr);
}

FileBase::OffsetType AppTextEditor::loadText(FileBase::OffsetType offset)
{
	file.setOffset(offset);

	size_t readed = 0;
	for (unsigned char i = 0; i < LineCount; i++)
	{
		readed = file.getLine(lineBuffers[i], LineSize - 1);
		lineBuffers[i][readed] = '\0';
		if (file.eof())
		{
			while (++i < LineCount)
				lineBuffers[i][0] = '\0';
		}
	}

	short nowY = lines[0].computeSize().y;
	for (unsigned char i = 1; i < LineCount; i++)
	{
		lines[i].position.y = nowY;
		nowY += lines[i].computeSize().y;
	}

	return file.getOffset();
}

void AppTextEditor::checkChange()
{
	// delete[] changeBuffer;
}

void AppTextEditor::clickCallback(unsigned char index)
{
	// changeBuffer = new char[LineSize + 1];
	// changeIndex = index;
	// strcpy(changeBuffer, lineBuffers[index]);
	// changeBuffer[LineSize - 1] = '\0';
	// AppInput* input = new AppInput{ lcd,touch, changeAppCallback, newAppCallback };
	// input->setInputBuffer(changeBuffer);
	// input->checker = [](char* buffer)->bool { return buffer[LineSize - 1] == '\0'; };
	// input->finishCallbackParam = this;
	// input->finishCallback = [](void* param) { AppTextEditor& self = *(AppTextEditor*)param; self.checkChange(); };
	// newAppCallback(input);
}

void AppTextEditor::click(Finger finger)
{
	contents.finger(finger);
}

void AppTextEditor::releaseDetect()
{
	if (touch[0].state != Finger::State::Contact && touch[1].state != Finger::State::Contact)
	{
		if (offset.x > 0)
		{
			offset.x = 0;
		}
		if (offset.y != 0)
		{
			if (offset.y > 50)
				fileOffset = loadText(0);
			else if (offset.y < -50)
				fileOffset = loadText(fileOffset);
			offset.y = 0;
		}
	}
}
