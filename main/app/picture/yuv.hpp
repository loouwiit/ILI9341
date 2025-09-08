#pragma once

#include "LCD/color.hpp"

class YUV
{
public:
	uint8_t y = 0;
	int8_t u = 0;
	int8_t v = 0;

	constexpr YUV() = default;
	constexpr YUV(const YUV&) = default;
	constexpr YUV& operator=(const YUV&) = default;
	constexpr YUV(YUV&&) = default;
	constexpr YUV& operator=(YUV&&) = default;

	constexpr YUV(uint8_t y, int8_t u, int8_t v) : y{ y }, u{ u }, v{ v } {}
	constexpr YUV(Color565 rgb) : YUV((Color666)rgb) {}
	constexpr YUV(Color666 rgb) :
		y{ (uint8_t)((77 * (rgb.R << 2) + 150 * (rgb.G << 2) + 29 * (rgb.B << 2)) >> 8) },
		u{ (int8_t)(((-44 * (rgb.R << 2) - 87 * (rgb.G << 2) + 131 * (rgb.B << 2)) >> 8)) },
		v{ (int8_t)(((131 * (rgb.R << 2) - 110 * (rgb.G << 2) - 21 * (rgb.B << 2)) >> 8)) }
	{}

	constexpr operator Color565() { return (Color565)(Color666)*this; }
	constexpr operator Color666()
	{
		return Color666
		{
			(uint8_t)((y + ((351 * v) >> 8)) >> 2),
			(uint8_t)((y - ((86 * u + 179 * v) >> 8)) >> 2),
			(uint8_t)((y + ((444 * u) >> 8)) >> 2)
		};
	}
};
