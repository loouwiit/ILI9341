#pragma once

#include "color.hpp"
#include "vector.hpp"
#include "drawable.hpp"

template <ColorTemplate Color, Vector2us Size>
class Line : public Drawable<Color, Size>
{
public:
	Vector2us start{};
	Vector2us end{};
	Color color{};

	Line() = default;
	Line(Vector2us start, Vector2us end, Color color = {}) : start{ start }, end{ end + Vector2us{1, 1} }, color{ color } {}
	Line(Line&) = default;
	Line& operator=(Line&) = default;
	Line(Line&&) = default;
	Line& operator=(Line&&) = default;

	virtual Vector2us drawTo(Drawable<Color, Size>::DrawTarget& target) override
	{
		using Vector2s = Vector2<signed short>;
		Vector2s delta = (Vector2s)end - (Vector2s)start;
		Vector2us absDelta = { (unsigned short)abs(delta.x), (unsigned short)abs(delta.y) };
		if (absDelta.x > absDelta.y)
		{
			Vector2f position = start;
			if (delta.x < 0)
			{
				position = end;
				delta = -delta;
			}

			float k = (float)delta.y / (float)delta.x;

			while ((unsigned short)position.x < end.x)
			{
				target[position] = color;
				position.x++;
				position.y += k;
			}
		}
		else
		{
			Vector2f position = start;
			if (delta.y < 0)
			{
				position = end;
				delta = -delta;
			}

			float l = (float)delta.x / (float)delta.y;

			while ((unsigned short)position.y < end.y)
			{
				target[position] = color;
				position.y++;
				position.x += l;
			}
		}
		return end - start;
	}
};
