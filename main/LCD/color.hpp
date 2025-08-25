#pragma once

#include <cstdint>
#include <algorithm>

class Color666
{
public:
	// XXRRRRRR
	uint8_t : 2;
	uint8_t R : 6;
	// XXGGGGGG
	uint8_t : 2;
	uint8_t G : 6;
	// XXBBBBBB
	uint8_t : 2;
	uint8_t B : 6;

	constexpr Color666() = default;
	constexpr Color666(const Color666&) = default;
	constexpr Color666& operator=(const Color666&) = default;
	constexpr Color666(Color666&&) = default;
	constexpr Color666& operator=(Color666&&) = default;

	constexpr Color666(const uint8_t R, const uint8_t G, const uint8_t B) :
		R{ R }, G{ G }, B{ B }
	{}

	bool operator==(Color666 o) { return R == o.R && G == o.G && B == o.B; }
	bool operator!=(Color666 o) { return R != o.R || G != o.G || B != o.B; }

	const static Color666 White;
	const static Color666 Black;
	const static Color666 Red;
	const static Color666 Green;
	const static Color666 Blue;
};

static_assert(sizeof(Color666) == 3, "666模式下Color应为3字节");

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
		GH{ (uint8_t)((G >> 3) & 0x7) },
		R{ (uint8_t)((R >> 1) & 0x1F) },
		B{ (uint8_t)((B >> 1) & 0x1F) },
		GL{ (uint8_t)(G & 0x7) }
	{}

	constexpr Color565(const uint16_t color) :
		GH{ (uint8_t)((color >> 8) & 0x7) },
		R{ (uint8_t)((color >> 11) & 0x1F) },
		B{ (uint8_t)(color & 0x1F) },
		GL{ (uint8_t)((color >> 5) & 0x7) }
	{}

	constexpr Color565(const Color666 color) :
		Color565(color.R, color.G, color.B)
	{}

	operator Color666() { return { (uint8_t)(R << 1), (uint8_t)((GH << 3) + GL), (uint8_t)(B << 1) }; }
	operator uint16_t() { return (R << 11) + (GH << 8) + (GL << 5) + B; }

	bool operator ==(Color565 o) { return GH == o.GH && R == o.R && B == o.B && GL == o.GL; }
	bool operator !=(Color565 o) { return GH != o.GH || R != o.R || B != o.B || GL != o.GL; }

	const static Color565 White;
	const static Color565 Black;
	const static Color565 Red;
	const static Color565 Green;
	const static Color565 Blue;
};
static_assert(sizeof(Color565) == 2, "565模式下Color应为2字节");

template <class T>
concept ColorTemplate = requires{std::is_same<T, Color565>::value || std::is_same<T, Color666>::value;};
