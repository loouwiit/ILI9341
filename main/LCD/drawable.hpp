#pragma once

#include "vector.hpp"
#include "frame.hpp"

template<ColorTemplate Color, Vector2us Size>
class Drawable
{
public:
	using DrawTarget = Frame<Color, Size>;

	virtual Vector2us drawTo(DrawTarget&) = 0;
	~Drawable() {};
};
