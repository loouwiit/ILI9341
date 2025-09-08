#pragma once

#include "color.hpp"

class YUV
{
public:
	unsigned char y = 0;
	signed char u = 0;
	signed char v = 0;

	constexpr YUV() = default;
	constexpr YUV(const YUV&) = default;
	constexpr YUV& operator=(const YUV&) = default;
	constexpr YUV(YUV&&) = default;
	constexpr YUV& operator=(YUV&&) = default;

	constexpr YUV(unsigned char y, signed char u, signed char v) : y{ y }, u{ u }, v{ v } {}
	constexpr YUV(Color565 rgb) : YUV((Color666)rgb) {}
	constexpr YUV(Color666 rgb) :
		y{ (unsigned char)((77 * (rgb.R << 2) + 150 * (rgb.G << 2) + 29 * (rgb.B << 2)) >> 8) },
		u{ (signed char)(((-44 * (rgb.R << 2) - 87 * (rgb.G << 2) + 131 * (rgb.B << 2)) >> 8)) },
		v{ (signed char)(((131 * (rgb.R << 2) - 110 * (rgb.G << 2) - 21 * (rgb.B << 2)) >> 8)) }
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
