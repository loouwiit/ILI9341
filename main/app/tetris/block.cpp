#include "block.hpp"
#include "map.hpp"

bool AppTetris::Block::check()
{
	for (int i = 0; i < 4;i++)
	{
		if (position[i].x < 0) return false;
		if (position[i].x >= MapSize.x) return false;

		if (position[i].y < 0) return false;
		if (position[i].y >= MapSize.y) return false;

		if (!map->isEmpty(position[i])) return false;
	}

	return true;
}

bool AppTetris::Block::move(Vector2s movement)
{
	for (int i = 0; i < 4; i++)
		position[i] += movement;

	if (!check())
	{
		for (int i = 0; i < 4; i++)
			position[i] -= movement;
		return false;
	}

	updateDraw();
	return true;
}

bool AppTetris::Block::turnLeft()
{
	if (type == BlockType::O) return true;

	//确定踢墙表
	const Vector2s(*RotatePosition)[BlockRotatePositionNumber] = nullptr;
	Vector2s rotateOffset = -BlockConstantOffset[(unsigned char)type][(unsigned char)rotation]; // 减去旧的固有偏移

	switch (rotation)
	{
		using Rotation::U, Rotation::D, Rotation::L, Rotation::R;
	default:
	case U:
	{
		rotation = L;
		rotateOffset += BlockConstantOffset[(unsigned char)type][(unsigned char)rotation]; // 加上新的固有偏移
		if (type == BlockType::I)
			RotatePosition = &BlockRotatePosition[(unsigned char)BlockRotatePositionType::IUL];
		else
			RotatePosition = &BlockRotatePosition[(unsigned char)BlockRotatePositionType::UL];
		break;
	}
	case L:
	{
		rotation = D;
		rotateOffset += BlockConstantOffset[(unsigned char)type][(unsigned char)rotation]; //加上新的固有偏移
		if (type == BlockType::I)
			RotatePosition = &BlockRotatePosition[(unsigned char)BlockRotatePositionType::ILD];
		else
			RotatePosition = &BlockRotatePosition[(unsigned char)BlockRotatePositionType::LD];
		break;
	}
	case D:
	{
		rotation = R;
		rotateOffset += BlockConstantOffset[(unsigned char)type][(unsigned char)rotation]; //加上新的固有偏移
		if (type == BlockType::I)
			RotatePosition = &BlockRotatePosition[(unsigned char)BlockRotatePositionType::IDR];
		else
			RotatePosition = &BlockRotatePosition[(unsigned char)BlockRotatePositionType::DR];
		break;
	}
	case R:
	{
		rotation = U;
		rotateOffset += BlockConstantOffset[(unsigned char)type][(unsigned char)rotation]; //加上新的固有偏移
		if (type == BlockType::I)
			RotatePosition = &BlockRotatePosition[(unsigned char)BlockRotatePositionType::IRU];
		else
			RotatePosition = &BlockRotatePosition[(unsigned char)BlockRotatePositionType::RU];
		break;
	}
	}

	// 保存
	Vector2s oldPosition[4]{};
	for (int i = 0; i < 4; i++)
		oldPosition[i] = position[i];

	//旋转
	Vector2s& center = oldPosition[1];
	Vector2s rotate[4]{};
	for (unsigned i = 0; i < 4; i++)
	{
		rotate[i].x = center.x - oldPosition[i].y + center.y + rotateOffset.x;
		rotate[i].y = center.y + oldPosition[i].x - center.x + rotateOffset.y;
	}

	//踢墙
	unsigned char rotatePositionIndex = 0;
	do
	{
		for (unsigned i = 0; i < 4; i++)
			position[i] = rotate[i] + (*RotatePosition)[rotatePositionIndex];
		rotatePositionIndex++;
	} while (rotatePositionIndex < BlockRotatePositionNumber && !check());

	// 检测
	if (check())
	{
		updateDraw();
		return true;
	}
	else
	{
		for (unsigned i = 0; i < 4; i++)
			position[i] = oldPosition[i];
		return false;
	}
}

bool AppTetris::Block::turnRight()
{
	if (type == BlockType::O) return true;

	//确定踢墙表
	const Vector2s(*RotatePosition)[BlockRotatePositionNumber] = nullptr;
	Vector2s rotateOffset = -BlockConstantOffset[(unsigned char)type][(unsigned char)rotation]; // 减去旧的固有偏移

	switch (rotation)
	{
		using Rotation::U, Rotation::D, Rotation::L, Rotation::R;
	default:
	case U:
	{
		rotation = R;
		rotateOffset += BlockConstantOffset[(unsigned char)type][(unsigned char)rotation]; // 加上新的固有偏移
		if (type == BlockType::I)
			RotatePosition = &BlockRotatePosition[(unsigned char)BlockRotatePositionType::IUR];
		else
			RotatePosition = &BlockRotatePosition[(unsigned char)BlockRotatePositionType::UR];
		break;
	}
	case R:
	{
		rotation = D;
		rotateOffset += BlockConstantOffset[(unsigned char)type][(unsigned char)rotation]; //加上新的固有偏移
		if (type == BlockType::I)
			RotatePosition = &BlockRotatePosition[(unsigned char)BlockRotatePositionType::IRD];
		else
			RotatePosition = &BlockRotatePosition[(unsigned char)BlockRotatePositionType::RD];
		break;
	}
	case D:
	{
		rotation = L;
		rotateOffset += BlockConstantOffset[(unsigned char)type][(unsigned char)rotation]; //加上新的固有偏移
		if (type == BlockType::I)
			RotatePosition = &BlockRotatePosition[(unsigned char)BlockRotatePositionType::IDL];
		else
			RotatePosition = &BlockRotatePosition[(unsigned char)BlockRotatePositionType::DL];
		break;
	}
	case L:
	{
		rotation = U;
		rotateOffset += BlockConstantOffset[(unsigned char)type][(unsigned char)rotation]; //加上新的固有偏移
		if (type == BlockType::I)
			RotatePosition = &BlockRotatePosition[(unsigned char)BlockRotatePositionType::ILU];
		else
			RotatePosition = &BlockRotatePosition[(unsigned char)BlockRotatePositionType::LU];
		break;
	}
	}

	// 保存
	Vector2s oldPosition[4]{};
	for (int i = 0; i < 4; i++)
		oldPosition[i] = position[i];

	//旋转
	Vector2s& center = oldPosition[1];
	Vector2s rotate[4]{};
	for (unsigned i = 0; i < 4; i++)
	{
		rotate[i].x = center.x + oldPosition[i].y - center.y + rotateOffset.x;
		rotate[i].y = center.y - oldPosition[i].x + center.x + rotateOffset.y;
	}

	//踢墙
	unsigned char rotatePositionIndex = 0;
	do
	{
		for (unsigned i = 0; i < 4; i++)
			position[i] = rotate[i] + (*RotatePosition)[rotatePositionIndex];
		rotatePositionIndex++;
	} while (rotatePositionIndex < BlockRotatePositionNumber && !check());

	// 检测
	if (check())
	{
		updateDraw();
		return true;
	}
	else
	{
		for (unsigned i = 0; i < 4; i++)
			position[i] = oldPosition[i];
		return false;
	}
}

void AppTetris::Block::lock()
{
	for (int i = 0; i < 4; i++)
		map->setBlock(position[i], block[i].color);
}

void AppTetris::Block::setBlockType(BlockType type)
{
	this->type = type;
	rotation = Rotation::U;

	for (int i = 0; i < 4; i++)
		block[i].color = BlockColor[(unsigned char)type];

	for (int i = 0; i < 4; i++)
	{
		position[i].x = ((RealMapSize.x - 1) / 2) - 1 + BlockPosition[(unsigned char)type][i] / 2;
		position[i].y = RealMapSize.y + 1 - BlockPosition[(unsigned char)type][i] % 2;
	}

	updateDraw();
}

BlockType AppTetris::Block::getBlockType()
{
	return type;
}

void AppTetris::Block::setBlockColor(LCD::Color color)
{
	for (int i = 0; i < 4; i++)
		block[i].color = color;
}

void AppTetris::Block::updateDraw()
{
	for (int i = 0; i < 4; i++)
	{
		block[i].start = (*map)[position[i]].start;
		block[i].end = (*map)[position[i]].end;
	}
}
