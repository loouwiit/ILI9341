#pragma once

#include "vector.hpp"
#include "frame.hpp"

template<ColorTemplate Color, Vector2s Size>
class Drawable
{
public:
	using DrawTarget = Frame<Color, Size>;

	virtual Vector2s drawTo(DrawTarget&, Vector2s offset = {}) = 0;
	~Drawable() {};
};
