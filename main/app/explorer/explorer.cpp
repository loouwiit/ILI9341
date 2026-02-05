#include "explorer.hpp"

#include "property.hpp"

#include <cstring>

#include "app/picture/picture.hpp"
#include "app/textEditor/textEditor.hpp"
#include "app/input/textInput.hpp"

void AppExplorer::init()
{
	App::init();

	contents[0] = &title;
	contents[1] = &pathLayar;
	contents[2] = &fileLayar;

	title.position.x = LCD::ScreenSize.x / 2;
	title.position.x -= title.computeSize().x / 2;
	title.computeSize();
	title.clickCallbackParam = this;
	title.releaseCallback = [](Finger&, void* param) { AppExplorer& self = *(AppExplorer*)param; self.back(); };

	pathLayar[0] = &path;
	pathLayar[1] = &newFile;

	path.text = nowFloorPath;
	path.computeSize();
	path.clickCallbackParam = this;
	path.releaseCallback = [](Finger&, void* param) { AppExplorer& self = *(AppExplorer*)param; self.updateFloor(); };

	newFile.position.x = path.getSize().x + GapSize;
	newFile.computeSize();
	newFile.clickCallbackParam = this;
	newFile.releaseCallback = [](Finger&, void* param)
		{
			auto& self = *(AppExplorer*)param;
			auto* app = new AppTextInput{ self.lcd, self.touch, self.changeAppCallback, self.newAppCallback };

			self.appTextInputBuffer = new char[256] {};
			app->setInputBuffer(self.appTextInputBuffer);
			app->checker = [](char* string)->bool
				{
					for (auto i = 0; i < 255;i++)
					{
						if (string[i] == '\0') return true;
						if (string[i] == '/' || string[i] == '\\')
						{
							if (string[i + 1] == '\0') return true;
							else return false;
						}
					}
					return true;
				};
			app->finishCallbackParam = param;
			app->finishCallback = [](void* param)
				{
					auto& self = *(AppExplorer*)param;
					auto& path = self.appTextInputBuffer;
					auto pathLength = strlen(path);

					if (path[pathLength - 1] == '/' || path[pathLength - 1] == '\\') do
					{
						// floor
						path[pathLength - 1] = '\0';
						pathLength--;
						if (pathLength == 0) break;

						char* buffer = new char[self.nowFloorPointer + 5 + pathLength + 1];
						strcpy(buffer, self.realFloorPath);
						buffer[self.nowFloorPointer + 5] = '/';
						strcpy(buffer + self.nowFloorPointer + 5, path);
						ESP_LOGI(TAG, "new floor %s", buffer);
						if (!testFloor(buffer)) newFloor(buffer);
						delete[] buffer;
					} while (false);
					else do
					{
						if (pathLength == 0) break;

						OFile file{};
						ESP_LOGI(TAG, "new file %s%s", self.realFloorPath, path);
						self.floor.openFile(path, pathLength, file); // 有底层封装就是好
					} while (false);
					delete[] self.appTextInputBuffer;
					self.appTextInputBuffer = nullptr;
					self.updateFloor();
				};

			self.newAppCallback(app);
		};

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
		files[i].holdCallback = [](Finger&, void* param)
			{
				auto& self = *((ClickCallbackParam_t*)param)->self;
				auto index = ((ClickCallbackParam_t*)param)->index;
				self.holdCallback(index);
			};
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

void AppExplorer::focusIn()
{
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
	delete titleBuffer;
	titleBuffer = nullptr;
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

	if (finger[0].state == Finger::State::Press) do
	{
		fingerActive[0] = true;
		fingerHoldTick[0] = xTaskGetTickCount() + holdTickThreshold;

		lastFingerPosition[0] = finger[0].position;
		fingerMoveTotol[0] = {};
		fingerActiveMovePath[0] = contents.isClicked(finger[0].position, path);
	} while (false);

	if (fingerActive[0]) do
	{
		auto movement = finger[0].position - lastFingerPosition[0];
		fingerMoveTotol[0] += movement;
		if (fingerActiveMovePath[0]) pathLayar.start.x += movement.x;
		else contents.start.y += movement.y;
		lastFingerPosition[0] = finger[0].position;
	} while (false);

	if (finger[0].state == Finger::State::Realease) do
	{
		if (abs2(fingerMoveTotol[0]) < moveThreshold2)
		{
			if (xTaskGetTickCount() > fingerHoldTick[0])
				click({ Finger::State::Hold, finger[0].position });
			else click(finger[0]);
		}
		fingerActive[0] = false;
		releaseDetect();
	} while (false);

	if (finger[1].state == Finger::State::Press) do
	{
		fingerActive[1] = true;
		fingerHoldTick[1] = xTaskGetTickCount() + holdTickThreshold;

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
		{
			if (xTaskGetTickCount() > fingerHoldTick[1])
				click({ Finger::State::Hold, finger[1].position });
			else click(finger[1]);
		}
		fingerActive[1] = false;
		releaseDetect();
	} while (false);
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

void AppExplorer::setTitle(const char* title)
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

void AppExplorer::setTitleBuffer(const char* titleBuffer)
{
	delete[] this->titleBuffer;
	this->titleBuffer = nullptr;

	this->title.text = titleBuffer != nullptr ? titleBuffer : defaultTitle;
	this->title.position.x = LCD::ScreenSize.x / 2;
	this->title.position.x -= this->title.computeSize().x / 2;
	this->title.computeSize();
}

void AppExplorer::resetPosition()
{
	contents.start.y = 0;
	pathLayar.start.x = ContentXOffset;
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
	newFile.position.x = path.computeSize().x + GapSize;
	newFile.computeSize();
	return true;
}

void AppExplorer::holdCallback(unsigned char index)
{
	auto* app = new AppProperty{ lcd, touch, changeAppCallback, newAppCallback };

	char* fileName = this->fileName[index];
	auto fileNameLength = strlen(fileName);
	auto floorLength = strlen(realFloorPath); // /root/xxx/floor/

	char* buffer = new char[fileNameLength + floorLength + 1]; // one for \0
	memcpy(buffer, realFloorPath, floorLength);
	memcpy(buffer + floorLength, fileName, fileNameLength + 1); // with \0
	app->setPath(buffer);

	delete[] buffer;

	newAppCallback(app);
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
	newFile.position.x = path.computeSize().x + GapSize;
	newFile.computeSize();
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
		if (contents.start.y > 0)
			contents.start.y = 0;
		if (pathLayar.start.x > ContentXOffset)
			pathLayar.start.x = ContentXOffset;
	}
}
