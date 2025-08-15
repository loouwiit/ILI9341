#pragma once

#include "color.hpp"
#include "vector.hpp"

// DMA_ATTR please
template <ColorTemplate Color, Vector2us Size>
class FrameBuffer
{
public:
	Color frame[Size.y][Size.x];

	constexpr Color* operator [](int i) { return frame[i]; }
	constexpr Color& operator [](Vector2us i) { return frame[i.y][i.x]; }

	constexpr Vector2us size() { return Size; }
	constexpr bool is565() { return std::is_same<Color, Color565>::value; }
	constexpr bool is666() { return std::is_same<Color, Color666>::value; }
};
