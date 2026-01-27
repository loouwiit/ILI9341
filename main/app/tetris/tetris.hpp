#pragma once

#include "app.hpp"
#include "time.h"

class AppTetris final : public App
{
public:
	AppTetris(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback) : App(lcd, touch, changeAppCallback, changeAppCallback) {}

	virtual void init() override final;
	virtual void deinit() override final;

	virtual void draw() override final;
	virtual void touchUpdate() override final;
	virtual void back() override final;

	constexpr static unsigned char BlockTypeCount = 7;
	enum class BlockType : unsigned char { Z, L, O, S, I, J, T, N = (unsigned char)-1 };

private:
	using Color = LCD::Color;

	class Map;
	Map* map = nullptr;
	class Block;
	Block* block = nullptr;
	Block* willBlock = nullptr;
	Block* nextBlocks = nullptr;

	constexpr static unsigned char NextBlocksCount = 8;

	constexpr static Color BottonTextBackgroundColor = { 0x20,0x20,0x20 };
	constexpr static unsigned char BottonTextScale = 3;
	constexpr static short GapSize = 8;
	constexpr static Vector2s BottonPosition = { 130,110 };
	constexpr static const char* const BottonText[6] = { "L", "/\\", "R", "<-", "\\/", "->" };
	LCD::Text botton[6]{};

	constexpr static clock_t PressGapTime = 300;
	constexpr static clock_t HoldGapTime = 50;
	clock_t nextHoldTime[6]{};

	bool survive = false;
	void restart();

	bool clockDown();
	bool softDown();
	bool hardDown();
	bool left();
	bool right();
	bool turnLeft();
	bool turnRight();

	constexpr static clock_t LockTimeMax = 5000;
	constexpr static clock_t LockTime = 2000;
	constexpr static clock_t Forever = -1;
	clock_t nextLockTime = Forever;
	clock_t nextLockTimeMax = Forever;

	void clearLock();
	void resetLock();
	void stretchLock();
	bool checkLock();

	constexpr static clock_t ClockDownTime = 1000;
	clock_t nextClockDownTime = Forever;

	void resetClockDownTime();
	static TickType_t downThread(void* param);
	bool downThreadRunning = false;

	void updateWillBlock();

	BlockType newNextBlocks[BlockTypeCount]{};
	unsigned char newNextBlockIndex = 0;
	void nextBlock();
	void randNewNextBlocks();

	LCD::Rectangle swapBlockRegin{};
	bool isBlockSwaped = false;
	void resetSwapBlock();
	bool swapBlock();
};
