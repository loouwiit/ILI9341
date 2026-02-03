#include "explorer.hpp"

#include <cstring>

#include "app/picture/picture.hpp"
#include "app/textEditor/textEditor.hpp"

void AppExplorer::init()
{
	App::init();

	contents[0] = &title;
	contents[1] = &path;
	contents[2] = &fileLayar;

	title.position.x -= title.computeSize().x / 2;
	title.computeSize();
	title.clickCallbackParam = this;
	title.releaseCallback = [](Finger&, void* param) { AppExplorer& self = *(AppExplorer*)param; self.back(); };

	path.text = nowFloorPath;
	path.computeSize();
	path.clickCallbackParam = this;
	path.releaseCallback = [](Finger&, void* param) { AppExplorer& self = *(AppExplorer*)param; self.updateFloor(); };

	fileLayar.start.x = ContentXOffset;
	fileLayar.start.y = 16 * TitleSize + GapSize + (16 * TextSize + GapSize) * 1;

	for (unsigned char i = 0; i < FileLayarSize; i++)
	{
		clickCallbackParam[i].self = this;
		clickCallbackParam[i].index = i;

		files[i].position.y = (16 * TextSize + GapSize) * i;
		files[i].textColor = LCD::Color::White;
		files[i].backgroundColor = BackgroundColor;
		files[i].scale = TextSize;
		files[i].text = fileName[i] = new char[256];
		files[i].clickCallbackParam = &clickCallbackParam[i];
		files[i].releaseCallback = [](Finger&, void* param)
			{
				auto& self = *((ClickCallbackParam_t*)param)->self;
				auto index = ((ClickCallbackParam_t*)param)->index;
				self.clickCallback(index);
			};

		fileLayar[i] = &files[i];
	}

	updateFloor();
}

void AppExplorer::deinit()
{
	running = false;
	for (unsigned char i = 0; i < FileLayarSize; i++)
	{
		delete[] fileName[i];
		files[i].text = fileName[i] = nullptr;
	}
	deleteAble = true;
}

void AppExplorer::draw()
{
	lcd.clear();
	lcd.draw(contents);
}

void AppExplorer::touchUpdate()
{
	Finger finger[2] = { touch[0],touch[1] };

	if (finger[0].state == Finger::State::Press)
	{
		pathMoveActive[0] = path.isClicked(finger[0].position);
		viewMoveActive[0] = !pathMoveActive[0];
		lastFingerPosition[0] = finger[0].position;
		fingerMoveTotol[0] = {};
	}
	else if (finger[0].state == Finger::State::Realease && (viewMoveActive[0] || pathMoveActive[0]))
	{
		if (abs2(fingerMoveTotol[0]) < moveThreshold2)
			click(finger[0]);
		if (viewMoveActive[0]) releaseDetect();
		pathMoveActive[0] = false;
		viewMoveActive[0] = false;
	}

	if (finger[1].state == Finger::State::Press)
	{
		pathMoveActive[1] = path.isClicked(finger[1].position);
		viewMoveActive[1] = !pathMoveActive[1];
		lastFingerPosition[1] = finger[1].position;
		fingerMoveTotol[1] = {};
	}
	else if (finger[1].state == Finger::State::Realease && (viewMoveActive[1] || pathMoveActive[1]))
	{
		if (abs2(fingerMoveTotol[1]) < moveThreshold2)
			click(finger[1]);
		if (viewMoveActive[1]) releaseDetect();
		pathMoveActive[1] = false;
		viewMoveActive[1] = false;
	}

	if (viewMoveActive[0] || pathMoveActive[0])
	{
		auto movement = finger[0].position - lastFingerPosition[0];
		fingerMoveTotol[0] += movement;
		if (viewMoveActive[0]) offset += movement.y;
		if (pathMoveActive[0]) { path.position.x += movement.x; path.computeSize(); }
		lastFingerPosition[0] = finger[0].position;
	}
	if (viewMoveActive[1] || pathMoveActive[1])
	{
		auto movement = finger[1].position - lastFingerPosition[1];
		fingerMoveTotol[1] += movement;
		if (viewMoveActive[1]) offset += movement.y;
		if (pathMoveActive[1]) { path.position.x += movement.x; path.computeSize(); }
		lastFingerPosition[1] = finger[1].position;
	}
}

void AppExplorer::back()
{
	if (!floorBack())
	{
		if (openFileCallback != nullptr)
			openFileCallback(nullptr, callBackParam);
		changeAppCallback(nullptr);
	}
}

void AppExplorer::resetPosition()
{
	offset = 0;
	path.position.x = ContentXOffset;
	path.computeSize();
}

void AppExplorer::updateFloor()
{
	floor.open(realFloorPath);

	floor.reCount();
	size_t floorCount = floor.getCount(Floor::Type::Floor);
	size_t fileCount = floor.getCount(Floor::Type::File);
	size_t totolCount = floor.getCount(Floor::Type::Both);
	if (floorCount > FileLayarSize)
	{
		floorCount = FileLayarSize - 1;
		fileCount = 0;
		totolCount = FileLayarSize;
		updateText(FileLayarSize - 1, "......", Floor::Type::Floor);
	}
	if (totolCount > FileLayarSize)
	{
		totolCount = FileLayarSize;
		fileCount = totolCount - floorCount;
		if (fileCount > 0) fileCount--;
		else floorCount--;
		updateText(FileLayarSize - 1, "......", Floor::Type::File);
	}

	for (size_t i = 0; i < floorCount; i++)
		updateText(i, floor.read(Floor::Type::Floor), Floor::Type::Floor);
	floor.backToBegin();
	for (size_t i = 0; i < fileCount; i++)
		updateText(floorCount + i, floor.read(Floor::Type::File), Floor::Type::File);
	fileLayar.elementCount = totolCount;
}

bool AppExplorer::floorBack()
{
	if (nowFloorPointer == 1)
	{
		updateFloor();
		return false;
	}

	nowFloorPointer--; // '/' <- '\0'
	do nowFloorPointer--;
	while (nowFloorPath[nowFloorPointer] != '/');
	nowFloorPointer++;
	nowFloorPath[nowFloorPointer] = '\0';
	resetPosition();
	updateFloor();
	return true;
}

void AppExplorer::clickCallback(unsigned char index)
{
	if (files[index].textColor != FloorColor)
	{
		// file
		openFile(index);
		return;
	}

	// floor
	auto length = strlen(fileName[index]);
	strcpy(nowFloorPath + nowFloorPointer, fileName[index]);
	nowFloorPointer += length;
	nowFloorPath[nowFloorPointer] = '/';
	nowFloorPointer++;
	nowFloorPath[nowFloorPointer] = '\0';
	floor.open(realFloorPath);
	resetPosition();
	updateFloor();
}

void AppExplorer::openFile(unsigned char index)
{
	char* fileName = this->fileName[index];
	auto fileNameLength = strlen(fileName);

	if (openFileCallback != nullptr)
	{
		auto floorLength = strlen(realFloorPath); // /root/xxx/floor/

		char* buffer = new char[fileNameLength + floorLength + 1]; // one for \0
		memcpy(buffer, realFloorPath, floorLength);
		memcpy(buffer + floorLength, fileName, fileNameLength + 1); // with \0

		openFileCallback(buffer, callBackParam);
		delete[] buffer;
		return;
	}

	if (strcmp(fileName + fileNameLength - 4, ".pic") == 0)
	{
		AppPicture* picture = new AppPicture{ lcd,touch, changeAppCallback, newAppCallback };
		floor.openFile(fileName, fileNameLength, picture->file);
		picture->fileName = fileName;
		if (picture->file.isOpen())
			newAppCallback(picture);
		else delete picture;
	}
	else
	{
		AppTextEditor* editor = new AppTextEditor{ lcd,touch, changeAppCallback, newAppCallback };
		floor.openFile(fileName, fileNameLength, editor->file);
		editor->fileName = fileName; // 生命周期安全，newApp不会析构过去的app
		newAppCallback(editor);
	}
}

void AppExplorer::updateText(unsigned char index, const char* text, Floor::Type type)
{
	strcpy(fileName[index], text);
	files[index].textColor = type == Floor::Type::Floor ? FloorColor : FileColor;
	files[index].computeSize();
}

void AppExplorer::click(Finger finger)
{
	contents.finger(finger);
}

void AppExplorer::releaseDetect()
{
	if (touch[0].state != Finger::State::Contact && touch[1].state != Finger::State::Contact)
	{
		if (offset > 0)
			offset = 0;
	}
}


