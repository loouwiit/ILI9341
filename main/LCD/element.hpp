#pragma once

#include "drawable.hpp"
#include "clickable.hpp"

template<ColorTemplate Color, Vector2us Size>
class Element : public Drawable<Color, Size>, public Clickable
{
};
