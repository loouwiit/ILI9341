#pragma once

#include <cstdint>
#include <algorithm>

class Color565
{
public:
	uint8_t GH : 3;
	uint8_t R : 5;
	uint8_t B : 5;
	uint8_t GL : 3;

	constexpr Color565() = default;
	constexpr Color565(const Color565&) = default;
	constexpr Color565& operator=(const Color565&) = default;
	constexpr Color565(Color565&&) = default;
	constexpr Color565& operator=(Color565&&) = default;

	constexpr Color565(const uint8_t R, const uint8_t G, const uint8_t B) :
		GL{ (uint8_t)(G & 0x7) },
		R{ (uint8_t)((R >> 1) & 0x1F) },
		B{ (uint8_t)((B >> 1) & 0x1F) },
		GH{ (uint8_t)((G >> 3) & 0x7) }
	{}

	operator uint16_t() { return (R << 11) + (GH << 8) + (GL << 5) + B; }

	bool operator ==(Color565 o) { return GH == o.GH && R == o.R && B == o.B && GL == o.GL; }
	bool operator !=(Color565 o) { return GH != o.GH || R != o.R || B != o.B || GL != o.GL; }
};
static_assert(sizeof(Color565) == 2, "565模式下Color应为2字节");
