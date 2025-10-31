#include "tetris.hpp"
#include "map.hpp"
#include "block.hpp"
#include "esp_random.h"

constexpr static char TAG[] = "AppTetris";

void AppTetris::init()
{
	App::init();

	if (map != nullptr || block != nullptr || willBlock != nullptr || nextBlocks != nullptr)
	{
		ESP_LOGE(TAG, "init but map = %p, block = %p, willBlock = %p blockBag = %p", map, block, willBlock, nextBlocks);
		delete map;
		delete block;
		delete willBlock;
		delete nextBlocks;
	}

	map = new Map{};
	block = new Block{};
	block->map = map;
	willBlock = new Block{};
	willBlock->map = map;

	nextBlocks = new Block[NextBlocksCount]{};
	auto blockSize = map->getBlockSize();
	for (int i = 0; i < NextBlocksCount; i++)
	{
		nextBlocks[i].map = map;
		nextBlocks[i].drawPosition = { (short)(blockSize * (9 + (i % 4) * 4.5f)), (short)(blockSize * (i / 4 * 4.5f)) };
	}
	swapBlockRegin.start = nextBlocks[0].drawPosition;
	swapBlockRegin.end = swapBlockRegin.start + Vector2s{ blockSize, blockSize } *6; // 判定稍微大一些

	for (int i = 0; i < BlockTypeCount; i++)
		newNextBlocks[i] = (BlockType)i;

	restart();

	if (pdTRUE == xTaskCreate(downThread, "tetris", 4096, this, 2, nullptr))
	{
		downThreadRunning = true;
	}
	else
	{
		ESP_LOGE(TAG, "out of memory, down thread creat failed");
		downThreadRunning = false;
	}

	botton[0].font = fontBuiltInFullWidth;
	botton[2].font = fontBuiltInFullWidth;

	Vector2s position = BottonPosition;
	for (int i = 0; i < 3; i++)
	{
		botton[i].position = position;
		botton[i].text = BottonText[i];
		botton[i].backgroundColor = BottonTextBackgroundColor;
		botton[i].scale = BottonTextScale;
		botton[i].clickCallbackParam = this;
		position.x += botton[i].computeSize().x + GapSize;
	}
	position = BottonPosition + Vector2s{ 0, (short)(botton[0].getSize().y + GapSize) };
	for (int i = 3; i < 6; i++)
	{
		botton[i].position = position;
		botton[i].text = BottonText[i];
		botton[i].backgroundColor = BottonTextBackgroundColor;
		botton[i].scale = BottonTextScale;
		botton[i].clickCallbackParam = this;
		position.x += botton[i].computeSize().x + GapSize;
	}

	botton[0].pressCallback = [](Finger&, void* param) { AppTetris& self = *(AppTetris*)param; if (self.survive) { self.nextHoldTime[0] = clock() + self.PressGapTime; self.turnLeft(); } };
	botton[1].pressCallback = [](Finger&, void* param) { AppTetris& self = *(AppTetris*)param; if (self.survive) { self.nextHoldTime[1] = clock() + self.PressGapTime; self.hardDown(); } };
	botton[2].pressCallback = [](Finger&, void* param) { AppTetris& self = *(AppTetris*)param; if (self.survive) { self.nextHoldTime[2] = clock() + self.PressGapTime; self.turnRight(); } };
	botton[3].pressCallback = [](Finger&, void* param) { AppTetris& self = *(AppTetris*)param; if (self.survive) { self.nextHoldTime[3] = clock() + self.PressGapTime; self.left(); } };
	botton[4].pressCallback = [](Finger&, void* param) { AppTetris& self = *(AppTetris*)param; if (self.survive) { self.nextHoldTime[4] = clock() + self.PressGapTime; self.softDown(); } };
	botton[5].pressCallback = [](Finger&, void* param) { AppTetris& self = *(AppTetris*)param; if (self.survive) { self.nextHoldTime[5] = clock() + self.PressGapTime; self.right(); } };

	botton[0].holdCallback = [](Finger&, void* param) { AppTetris& self = *(AppTetris*)param; if (self.survive && self.nextHoldTime[0] < clock()) { self.nextHoldTime[0] = clock() + self.HoldGapTime; self.turnLeft(); } };
	// botton[1].holdCallback = [](Finger&, void* param) { AppTetris& self = *(AppTetris*)param; if (self.survive && self.nextHoldTime[1] < clock()) { self.nextHoldTime[1] = clock() + self.HoldGapTime; self.hardDown(); } };
	botton[2].holdCallback = [](Finger&, void* param) { AppTetris& self = *(AppTetris*)param; if (self.survive && self.nextHoldTime[2] < clock()) { self.nextHoldTime[2] = clock() + self.HoldGapTime; self.turnRight(); } };
	botton[3].holdCallback = [](Finger&, void* param) { AppTetris& self = *(AppTetris*)param; if (self.survive && self.nextHoldTime[3] < clock()) { self.nextHoldTime[3] = clock() + self.HoldGapTime; self.left(); } };
	botton[4].holdCallback = [](Finger&, void* param) { AppTetris& self = *(AppTetris*)param; if (self.survive && self.nextHoldTime[4] < clock()) { self.nextHoldTime[4] = clock() + self.HoldGapTime; self.softDown(); } };
	botton[5].holdCallback = [](Finger&, void* param) { AppTetris& self = *(AppTetris*)param; if (self.survive && self.nextHoldTime[5] < clock()) { self.nextHoldTime[5] = clock() + self.HoldGapTime; self.right(); } };

	swapBlockRegin.clickCallbackParam = this;
	swapBlockRegin.pressCallback = [](Finger&, void* param) { AppTetris& self = *(AppTetris*)param; if (self.survive) self.swapBlock(); };

	map->clickCallbackParam = this;
	map->pressCallback = [](Finger&, void* param) { AppTetris& self = *(AppTetris*)param; if (!self.survive) self.restart(); };
}

void AppTetris::deinit()
{
	running = false;
	while (downThreadRunning) vTaskDelay(1);

	delete map;
	map = nullptr;

	delete block;
	block = nullptr;

	delete willBlock;
	willBlock = nullptr;

	deleteAble = true;
}

void AppTetris::draw()
{
	lcd.clear();
	lcd.draw(*map);
	lcd.draw(*willBlock);
	lcd.draw(*block);

	for (int i = 0; i < NextBlocksCount; i++)
		lcd.draw(nextBlocks[i]);

	for (int i = 0; i < 6; i++)
		lcd.draw(botton[i]);
}

void AppTetris::touchUpdate()
{
	map->finger(touch[0]);
	map->finger(touch[1]);

	swapBlockRegin.finger(touch[0]);
	swapBlockRegin.finger(touch[1]);

	for (int i = 0; i < 6; i++)
	{
		botton[i].finger(touch[0]);
		botton[i].finger(touch[1]);
	}
}

void AppTetris::back()
{
	changeAppCallback(nullptr);
}

void AppTetris::restart()
{
	resetClockDownTime();
	map->clear();

	randNewNextBlocks();
	for (int i = 0; i < NextBlocksCount; i++)
	{
		if (newNextBlockIndex >= BlockTypeCount)
			randNewNextBlocks();
		nextBlocks[i].setBlockType(newNextBlocks[newNextBlockIndex]);
		newNextBlockIndex++;
	}
	nextBlock();

	updateWillBlock();
	resetClockDownTime();
	resetSwapBlock();
	clearLock();
	survive = true;
}

bool AppTetris::clockDown()
{
	resetClockDownTime();
	if (block->move({ 0,-1 }))
	{
		clearLock();
		return true;
	}

	if (nextLockTimeMax == Forever)
		resetLock();

	checkLock();

	return false;
}

bool AppTetris::softDown()
{
	if (block->move({ 0,-1 }))
	{
		resetClockDownTime();
		stretchLock();
		return true;
	}

	checkLock();
	return false;
}

bool AppTetris::hardDown()
{
	resetClockDownTime();

	while (block->move({ 0,-1 })) {}

	nextLockTime = 0; // 直接锁定
	checkLock();

	return true;
}

bool AppTetris::left()
{
	if (block->move({ -1,0 }))
	{
		stretchLock();
		updateWillBlock();
		return true;
	}
	return false;
}

bool AppTetris::right()
{
	if (block->move({ 1,0 }))
	{
		stretchLock();
		updateWillBlock();
		return true;
	}
	return false;
}

bool AppTetris::turnLeft()
{
	if (block->turnLeft())
	{
		updateWillBlock();
		stretchLock();
		return true;
	}
	return false;
}

bool AppTetris::turnRight()
{
	if (block->turnRight())
	{
		updateWillBlock();
		stretchLock();
		return true;
	}
	return false;
}

void AppTetris::clearLock()
{
	nextLockTimeMax = Forever;
	nextLockTime = Forever;
}

void AppTetris::resetLock()
{
	clock_t nowTime = clock();
	nextLockTimeMax = nowTime + LockTimeMax;
	nextLockTime = nowTime + LockTime;
}

void AppTetris::stretchLock()
{
	nextLockTime = clock() + LockTime;
	if (nextLockTimeMax < nextLockTime)
		nextLockTime = nextLockTimeMax;
}

bool AppTetris::checkLock()
{
	if (nextLockTime < clock())
	{
		block->lock();

		// 检测消行
		{
			short yMin = 0;
			short yMax = 0;
			for (int i = 0; i < 4; i++)
			{
				short y = block->getPosition(i).y;
				yMin = std::min(y, yMin);
				yMax = std::max(y, yMax);
			}

			for (short y = yMax; y >= yMin; y--) // 倒着走不用担心错行
				if (map->detectLineFull(y))
					map->removeLine(y);
		}

		// 下一个方块
		nextBlock();
		updateWillBlock();
		resetClockDownTime();
		resetSwapBlock();
		clearLock();

		if (!block->check())
		{
			survive = false;
			block->setBlockColor(GrayColor);
		}

		return true;
	}
	else [[likely]] return false;
}

void AppTetris::resetClockDownTime()
{
	nextClockDownTime = clock() + ClockDownTime;
}

void AppTetris::downThread(void* param)
{
	AppTetris& self = *(AppTetris*)param;
	self.nextClockDownTime = clock() + self.ClockDownTime;
	while (self.running)
	{
		if (!self.survive)
		{
			vTaskDelay(1000);
			continue;
		}

		// 没有锁，可能有bug
		if (self.nextClockDownTime < clock()) self.clockDown();
		vTaskDelay(10);
	}
	self.downThreadRunning = false;
	vTaskDelete(nullptr);
}

void AppTetris::updateWillBlock()
{
	*willBlock = *block;
	willBlock->setBlockColor(WillBlockColor[(int)willBlock->getBlockType()]);
	while (willBlock->move({ 0,-1 })) {}
}

void AppTetris::nextBlock()
{
	block->setBlockType(nextBlocks[0].getBlockType());
	for (int i = 0; i < NextBlocksCount - 1; i++)
		nextBlocks[i].setBlockType(nextBlocks[i + 1].getBlockType());
	if (newNextBlockIndex >= BlockTypeCount)
		randNewNextBlocks();
	nextBlocks[NextBlocksCount - 1].setBlockType(newNextBlocks[newNextBlockIndex]);
	newNextBlockIndex++;
}

void AppTetris::randNewNextBlocks()
{
	BlockType* arrayMoved = newNextBlocks;
	arrayMoved++;
	auto lengthSub = BlockTypeCount - 1;
	std::swap(arrayMoved[-1], arrayMoved[esp_random() % (lengthSub - 1)]); // 第一个和最后一个不能相同

	for (int i = 0; i < lengthSub; i++)
	{
		std::swap(arrayMoved[i], arrayMoved[esp_random() % lengthSub]);
	}
	newNextBlockIndex = 0;
}

void AppTetris::resetSwapBlock()
{
	isBlockSwaped = false;
}

bool AppTetris::swapBlock()
{
	if (isBlockSwaped) return false;
	isBlockSwaped = true;

	auto nowBlockType = block->getBlockType();
	block->setBlockType(nextBlocks[0].getBlockType());
	nextBlocks[0].setBlockType(nowBlockType);

	updateWillBlock();
	resetClockDownTime();
	clearLock();

	if (!block->check())
	{
		survive = false;
		block->setBlockColor(GrayColor);
	}

	return true;
}
