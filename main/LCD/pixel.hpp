#pragma once

#include "color.hpp"
#include "vector.hpp"
#include "drawable.hpp"

template <ColorTemplate Color, Vector2us Size>
class Pixel : public Drawable<Color, Size>
{
public:
	Pixel() = default;
	Pixel(Vector2s position, Color color = {}) : position{ position }, color{ color } {}
	Pixel(Pixel&) = default;
	Pixel& operator=(Pixel&) = default;
	Pixel(Pixel&&) = default;
	Pixel& operator=(Pixel&&) = default;

	Vector2s position{};
	Color color{};

	virtual Vector2us drawTo(Drawable<Color, Size>::DrawTarget& target, Vector2s offset = {}) override
	{
		Vector2s drawPosition = position + offset;
		if (drawPosition.x < 0) return { 1,1 };
		if (drawPosition.y < 0) return { 1,1 };
		if (drawPosition.x >= Size.x) return { 1,1 };
		if (drawPosition.y >= Size.y) return { 1,1 };
		target[drawPosition] = color;
		return { 1,1 };
	}
};
