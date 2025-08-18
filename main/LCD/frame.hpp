#pragma once

#include "color.hpp"
#include "vector.hpp"

// DMA_ATTR please
template <ColorTemplate Color, Vector2us Size>
class Frame
{
public:
	Color buffer[Size.y][Size.x];

	Vector2us draw(auto& element)
	{
		return element.drawTo(*this);
	}

	constexpr Color* operator [](int i) { return buffer[i]; }
	constexpr Color& operator [](Vector2s i) { return buffer[i.y][i.x]; }

	constexpr bool is565() { return std::is_same<Color, Color565>::value; }
	constexpr bool is666() { return std::is_same<Color, Color666>::value; }
};
