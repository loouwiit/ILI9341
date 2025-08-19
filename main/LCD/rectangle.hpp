#pragma once

#include "color.hpp"
#include "vector.hpp"
#include "element.hpp"

template <ColorTemplate Color, Vector2us Size>
class Rectangle : public Element<Color, Size>
{
public:
	Vector2s start{};
	Vector2s end{};
	Color color{};

	Rectangle() = default;
	Rectangle(Vector2s start, Vector2s size, Color color = {}) : start{ start }, end{ start + size }, color{ color } {}
	Rectangle(Rectangle&) = default;
	Rectangle& operator=(Rectangle&) = default;
	Rectangle(Rectangle&&) = default;
	Rectangle& operator=(Rectangle&&) = default;

	Vector2us getSize()
	{
		return end - start;
	}

	virtual bool isClicked(Vector2s point) override final
	{
		return start.x <= point.x && point.x < end.x &&
			start.y <= point.y && point.y < end.y;
	}

	virtual Vector2us drawTo(Drawable<Color, Size>::DrawTarget& target, Vector2s offset = {}) override
	{
		Vector2s drawStart = start + offset;
		Vector2s drawEnd = end + offset;

		if (drawStart.x < 0) drawStart.x = 0;
		if (drawStart.y < 0) drawStart.y = 0;
		if (drawStart.x > Size.x) drawStart.x = Size.x;
		if (drawStart.y > Size.y) drawStart.y = Size.y;

		if (drawEnd.x < 0) drawEnd.x = 0;
		if (drawEnd.y < 0) drawEnd.y = 0;
		if (drawEnd.x > Size.x) drawEnd.x = Size.x;
		if (drawEnd.y > Size.y) drawEnd.y = Size.y;

		for (unsigned short y = drawStart.y; y < drawEnd.y; y++)
			std::fill(&target[y][drawStart.x], &target[y][drawEnd.x], color);
		return end - start;
	}
};
