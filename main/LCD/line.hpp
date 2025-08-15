#pragma once

#include "vector.hpp"
#include "color.hpp"
#include "frame.hpp"

template <ColorTemplate Color>
class Line
{
public:
	Vector2us start{};
	Vector2us end{};
	Color color{};

	Line() = default;
	Line(Vector2us start, Vector2us end, Color color = {}) : start{ start }, end{ end }, color{ color } {}
	Line(Line&) = default;
	Line& operator=(Line&) = default;
	Line(Line&&) = default;
	Line& operator=(Line&&) = default;

	template <Vector2us Size>
	Vector2us drawTo(FrameBuffer<Color, Size>& target)
	{
		using Vector2s = Vector2<signed short>;
		Vector2s delta = (Vector2s)end - (Vector2s)start;
		Vector2us absDelta = { (unsigned char)abs(delta.x), (unsigned char)abs(delta.y) };
		if (absDelta.x > absDelta.y)
		{
			if (delta.x < 0)
			{
				start.swap(end);
				delta = -delta;
			}

			float k = (float)delta.y / (float)delta.x;

			Vector2f position = start;
			while ((int)position.x <= end.x)
			{
				target[position] = color;
				position.x++;
				position.y += k;
			}
		}
		else
		{
			if (delta.y < 0)
			{
				start.swap(end);
				delta = -delta;
			}

			float l = (float)delta.x / (float)delta.y;

			Vector2f position = start;
			while ((int)position.y <= end.y)
			{
				target[position] = color;
				position.y++;
				position.x += l;
			}
		}
		return delta;
	}
};
