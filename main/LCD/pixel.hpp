#pragma once

#include "color.hpp"
#include "vector.hpp"
#include "drawable.hpp"

template <ColorTemplate Color, Vector2us Size>
class Pixel : public Drawable<Color, Size>
{
public:
	Pixel() = default;
	Pixel(Vector2us position, Color color = {}) : position{ position }, color{ color } {}
	Pixel(Pixel&) = default;
	Pixel& operator=(Pixel&) = default;
	Pixel(Pixel&&) = default;
	Pixel& operator=(Pixel&&) = default;

	Vector2us position{};
	Color color{};

	virtual Vector2us drawTo(Drawable<Color, Size>::DrawTarget& target) override
	{
		target[position] = color;
		return { 1,1 };
	}
};
