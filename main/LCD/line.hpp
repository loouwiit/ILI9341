#pragma once

#include "color.hpp"
#include "vector.hpp"
#include "drawable.hpp"

template <ColorTemplate Color, Vector2us Size>
class Line : public Drawable<Color, Size>
{
public:
	Vector2s start{};
	Vector2s end{};
	Color color{};

	Line() = default;
	Line(Vector2s start, Vector2s end, Color color = {}) : start{ start }, end{ end + Vector2s{1, 1} }, color{ color } {}
	Line(Line&) = default;
	Line& operator=(Line&) = default;
	Line(Line&&) = default;
	Line& operator=(Line&&) = default;

	virtual Vector2us drawTo(Drawable<Color, Size>::DrawTarget& target, Vector2s offset = {}) override
	{
		using Vector2s = Vector2<signed short>;
		Vector2s delta = (Vector2s)end - (Vector2s)start;
		if (abs(delta.x) >abs(delta.y))
		{
			Vector2f drawPosition = start + offset;
			Vector2s drawEnd = end + offset;
			if (delta.x < 0)
			{
				drawPosition = drawEnd;
				delta = -delta;
			}

			float k = (float)delta.y / (float)delta.x;

			while ((unsigned short)drawPosition.x < drawEnd.x)
			{
				target[drawPosition] = color;
				drawPosition.x++;
				drawPosition.y += k;
			}
		}
		else
		{
			Vector2f drawPosition = start + offset;
			Vector2s drawEnd = end + offset;
			if (delta.y < 0)
			{
				drawPosition = drawEnd;
				delta = -delta;
			}

			float l = (float)delta.x / (float)delta.y;

			while ((unsigned short)drawPosition.y < drawEnd.y)
			{
				target[drawPosition] = color;
				drawPosition.y++;
				drawPosition.x += l;
			}
		}
		return end - start;
	}
};
