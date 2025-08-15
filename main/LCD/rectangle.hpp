#pragma once

#include "vector.hpp"
#include "color.hpp"
#include "frame.hpp"

template <ColorTemplate Color>
class Rectangle
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

	template <Vector2us Size>
	Vector2us drawTo(FrameBuffer<Color, Size>& target)
	{
		for (unsigned short i = start.y; i < end.y; i++)
			std::fill(&target[i][start.x], &target[i][end.x] + 1, color);
		return end - start;
	}
};
