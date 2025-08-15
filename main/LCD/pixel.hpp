#pragma once

#include "drawable.hpp"
#include "vector.hpp"
#include "color.hpp"
#include "frame.hpp"

template <ColorTemplate Color>
class Pixel : public Drawable<Pixel<Color>>
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

	template <Vector2us FrameSize>
	Vector2us drawTo(Frame<Color, FrameSize>& target)
	{
		target[position] = color;
		return { 1,1 };
	}
};
