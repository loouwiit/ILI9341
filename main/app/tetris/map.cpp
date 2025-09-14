#include "map.hpp"

AppTetris::Map::Map()
{
	setBlockSize(blockSize);
	clear();
}

AppTetris::Map::~Map()
{}

void AppTetris::Map::setBlockSize(unsigned char blockSize)
{
	this->blockSize = blockSize;
	size = MapSize * blockSize;

	for (unsigned char y = 0; y < MapSize.y; y++) for (unsigned char x = 0; x < MapSize.x; x++)
		block[MapSize.y - 1 - y][x].start = { (short)(blockSize * x), (short)(blockSize * y) };

	for (unsigned char y = 0; y < MapSize.y; y++) for (unsigned char x = 0; x < MapSize.x; x++)
		block[y][x].end = block[y][x].start + Vector2s{ (short)(blockSize - 1), (short)(blockSize - 1) };
}

void AppTetris::Map::clear()
{
	for (unsigned char y = 0; y < RealMapSize.y; y++) for (unsigned char x = 0; x < RealMapSize.x; x++)
		block[y][x].color = GrayColor;

	for (unsigned char y = RealMapSize.y; y < MapSize.y; y++) for (unsigned char x = 0; x < MapSize.x; x++)
		block[y][x].color = BackGroundColor;
}
