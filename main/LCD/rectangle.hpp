#pragma once

#include "color.hpp"
#include "vector.hpp"
#include "element.hpp"

template <ColorTemplate Color, Vector2us Size>
class Rectangle : public Element<Color, Size>
{
public:
	Vector2us start{};
	Vector2us end{};
	Color color{};

	Rectangle() = default;
	Rectangle(Vector2us start, Vector2us size, Color color = {}) : start{ start }, end{ start + size }, color{ color } {}
	Rectangle(Rectangle&) = default;
	Rectangle& operator=(Rectangle&) = default;
	Rectangle(Rectangle&&) = default;
	Rectangle& operator=(Rectangle&&) = default;

	Vector2us getSize()
	{
		return end - start;
	}

	virtual bool isClicked(Vector2us point) override final
	{
		return start.x <= point.x && point.x < end.x &&
			start.y <= point.y && point.y < end.y;
	}

	virtual Vector2us drawTo(Drawable<Color, Size>::DrawTarget& target) override
	{
		for (unsigned short y = start.y; y < end.y; y++)
			std::fill(&target[y][start.x], &target[y][end.x], color);
		return end - start;
	}
};
