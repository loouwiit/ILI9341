#pragma once

#include "LCD/drawable.hpp"
#include "tetris.hpp"
#include "mapBase.hpp"

class AppTetris::Map final : public Element<App::LCD::Color, App::LCD::ScreenSize>
{
public:
	Vector2s position{};

	Map();
	~Map();

	virtual Vector2us drawTo(DrawTarget& target, Vector2s offset = {}) override final
	{
		offset += position;
		for (unsigned char y = 0; y < MapSize.y; y++) for (unsigned char x = 0; x < MapSize.x; x++)
			block[y][x].drawTo(target, offset);
		return size;
	}

	virtual bool isClicked(Vector2s point) override final
	{
		point -= position;
		return 0 <= point.x && point.x < size.x
			&& 0 <= point.y && point.y < size.y;
	}

	unsigned char getBlockSize() { return blockSize; }
	void setBlockSize(unsigned char blockSize);

	bool isEmpty(Vector2s index) { return block[index.y][index.x].color == GrayColor || block[index.y][index.x].color == BackGroundColor; }
	LCD::Color getBlock(Vector2s index) { return block[index.y][index.x].color; }
	void setBlock(Vector2s index, LCD::Color color) { block[index.y][index.x].color = color; }
	void setBlock(Vector2s index, unsigned char colorIndex) { block[index.y][index.x].color = BlockColor[colorIndex]; }
	void clearBlock(Vector2s index) { block[index.y][index.x].color = index.y < MapSize.y - 4 ? GrayColor : BackGroundColor; }

	void clear();

	bool detectLineFull(short y)
	{
		for (unsigned char x = 0; x < MapSize.x; x++)
			if (isEmpty({ x, y })) return false;

		return true;
	}

	void removeLine(short y)
	{
		for (; y < RealMapSize.y - 1; y++) for (unsigned char x = 0; x < MapSize.x; x++)
			block[y][x].color = block[y + 1][x].color;

		// MapSize.y - 1 没有移动对象
		for (unsigned char x = 0; x < MapSize.x; x++)
			block[MapSize.y - 1][x].color = BackGroundColor;

		// RealMapSize.y - 1 的颜色不同
		for (unsigned char x = 0; x < MapSize.x; x++)
			if (block[RealMapSize.y - 1][x].color == BackGroundColor) block[RealMapSize.y - 1][x].color = GrayColor;
	}

protected:
	friend class AppTetris::Block;
	LCD::Rectangle operator[](Vector2s index) { return block[index.y][index.x]; }

private:
	unsigned char blockSize = 10;
	Vector2s size = MapSize * blockSize;

	LCD::Rectangle block[MapSize.y][MapSize.x]{};
};
