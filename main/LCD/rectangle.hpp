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
		for (unsigned short y = drawStart.y; y < drawEnd.y; y++)
			std::fill(&target[y][drawStart.x], &target[y][drawEnd.x], color);
		return drawEnd - drawStart;
	}
};
