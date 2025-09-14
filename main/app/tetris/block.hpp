#pragma once

#include "LCD/drawable.hpp"
#include "tetris.hpp"
#include "mapBase.hpp"

class AppTetris::Block final : public Drawable<App::LCD::Color, App::LCD::ScreenSize>
{
public:
	Vector2s drawPosition{};
	AppTetris::Map* map = nullptr;

	bool check();
	bool move(Vector2s movement);
	bool turnLeft();
	bool turnRight();

	Vector2s getPosition(int index) { return position[index]; }

	void lock();
	void setBlockType(BlockType type);
	BlockType getBlockType();

	virtual Vector2us drawTo(DrawTarget& target, Vector2s offset = {}) override final
	{
		offset += drawPosition;
		for (int i = 0; i < 4; i++)
			block[i].drawTo(target, offset);

		return MapSize * blockSize;
	}

protected:
	friend class AppTetris;
	void setBlockColor(LCD::Color color);

private:
	BlockType type = BlockType::O;
	Rotation rotation = Rotation::U;
	Vector2s position[4]{};

	unsigned char blockSize = 10;
	LCD::Rectangle block[4]{};

	void updateDraw();
};
