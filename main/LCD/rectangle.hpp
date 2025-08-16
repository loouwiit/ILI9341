#pragma once

#include "color.hpp"
#include "vector.hpp"
#include "drawable.hpp"

template <ColorTemplate Color, Vector2us Size>
class Rectangle : public Drawable<Color, Size>
{
public:
	Vector2us start{};
	Vector2us end{};
	Color color{};

	Rectangle() = default;
	Rectangle(Vector2us start, Vector2us end, Color color = {}) : start{ start }, end{ end }, color{ color } {}
	Rectangle(Rectangle&) = default;
	Rectangle& operator=(Rectangle&) = default;
	Rectangle(Rectangle&&) = default;
	Rectangle& operator=(Rectangle&&) = default;

	virtual Vector2us drawTo(Drawable<Color, Size>::DrawTarget& target) override
	{
		for (unsigned short i = start.y; i <= end.y; i++)
			std::fill(&target[i][start.x], &target[i][end.x] + 1, color);
		return end - start + Vector2us{1, 1};
	}
};
