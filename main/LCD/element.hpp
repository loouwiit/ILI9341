#pragma once

#include "drawable.hpp"
#include "clickable.hpp"

template<ColorTemplate Color, Vector2s Size>
class Element : public Drawable<Color, Size>, public Clickable
{
};
