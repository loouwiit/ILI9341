#pragma once

#include <cstdint>
#include <algorithm>

class Color888
{
public:
	// RRRRRRRR
	uint8_t R = 0;
	// GGGGGGGG
	uint8_t G = 0;
	// BBBBBBBB
	uint8_t B = 0;

	constexpr Color888() = default;
	constexpr Color888(const Color888&) = default;
	constexpr Color888& operator=(const Color888&) = default;
	constexpr Color888(Color888&&) = default;
	constexpr Color888& operator=(Color888&&) = default;

	constexpr Color888(const uint8_t R, const uint8_t G, const uint8_t B) :
		R{ R }, G{ G }, B{ B }
	{}

	constexpr bool operator==(Color888 o) { return R == o.R && G == o.G && B == o.B; }
	constexpr bool operator!=(Color888 o) { return R != o.R || G != o.G || B != o.B; }

	constexpr Color888 operator+(Color888 o) { return { (uint8_t)(R + o.R), (uint8_t)(G + o.G), (uint8_t)(B + o.B) }; }
	constexpr Color888 operator-(Color888 o) { return { (uint8_t)(R - o.R), (uint8_t)(G - o.G), (uint8_t)(B - o.B) }; }

	const static Color888 White;
	const static Color888 Black;
	const static Color888 Red;
	const static Color888 Green;
	const static Color888 Blue;
};

static_assert(sizeof(Color888) == 3, "888模式下Color应为3字节");

class Color666
{
public:
	// XXRRRRRR
	uint8_t : 2;
	uint8_t R : 6 = 0;
	// XXGGGGGG
	uint8_t : 2;
	uint8_t G : 6 = 0;
	// XXBBBBBB
	uint8_t : 2;
	uint8_t B : 6 = 0;

	constexpr Color666() = default;
	constexpr Color666(const Color666&) = default;
	constexpr Color666& operator=(const Color666&) = default;
	constexpr Color666(Color666&&) = default;
	constexpr Color666& operator=(Color666&&) = default;

	constexpr Color666(const uint8_t R, const uint8_t G, const uint8_t B) :
		R{ (uint8_t)(R >> 2) }, G{ (uint8_t)(G >> 2) }, B{ (uint8_t)(B >> 2) }
	{}

	constexpr Color666(const Color888 color) :
		Color666(color.R, color.G, color.B)
	{}

	constexpr operator Color888()
	{
		return { (uint8_t)(R << 2),(uint8_t)(G << 2),(uint8_t)(B << 2) };
	}

	constexpr bool operator==(Color666 o) { return R == o.R && G == o.G && B == o.B; }
	constexpr bool operator!=(Color666 o) { return R != o.R || G != o.G || B != o.B; }

	constexpr Color666 operator+(Color666 o) { return { (unsigned char)(R + o.R), (unsigned char)(G + o.G), (unsigned char)(B + o.B) }; }
	constexpr Color666 operator-(Color666 o) { return { (unsigned char)(R - o.R), (unsigned char)(G - o.G), (unsigned char)(B - o.B) }; }

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
	uint8_t GH : 3 = 0;
	uint8_t R : 5 = 0;
	uint8_t B : 5 = 0;
	uint8_t GL : 3 = 0;

	constexpr Color565() = default;
	constexpr Color565(const Color565&) = default;
	constexpr Color565& operator=(const Color565&) = default;
	constexpr Color565(Color565&&) = default;
	constexpr Color565& operator=(Color565&&) = default;

	constexpr Color565(const uint8_t R, const uint8_t G, const uint8_t B) :
		GH{ (uint8_t)((G >> 5) & 0x7) },
		R{ (uint8_t)((R >> 3) & 0x1F) },
		B{ (uint8_t)((B >> 3) & 0x1F) },
		GL{ (uint8_t)((G >> 2) & 0x7) }
	{}

	constexpr Color565(const uint16_t color) :
		GH{ (uint8_t)((color >> 8) & 0x7) },
		R{ (uint8_t)((color >> 11) & 0x1F) },
		B{ (uint8_t)(color & 0x1F) },
		GL{ (uint8_t)((color >> 5) & 0x7) }
	{}

	constexpr Color565(const Color888 color) :
		Color565(color.R, color.G, color.B)
	{}

	constexpr Color565(const Color666 color) :
		Color565(color.R << 2, color.G << 2, color.B << 2)
	{}

	constexpr operator Color888() { return { (uint8_t)(R << 3), (uint8_t)((GH << 5) | (GL << 2)), (uint8_t)(B << 3) }; }
	constexpr operator Color666() { return { (uint8_t)(R << 3), (uint8_t)((GH << 5) | (GL << 2)), (uint8_t)(B << 3) }; }
	constexpr operator uint16_t() { return (R << 11) | (GH << 8) | (GL << 5) | B; }

	constexpr bool operator ==(Color565 o) { return GH == o.GH && R == o.R && B == o.B && GL == o.GL; }
	constexpr bool operator !=(Color565 o) { return GH != o.GH || R != o.R || B != o.B || GL != o.GL; }

	uint8_t getG() { return (uint8_t)((GH << 3) + GL); }

	const static Color565 White;
	const static Color565 Black;
	const static Color565 Red;
	const static Color565 Green;
	const static Color565 Blue;
};
static_assert(sizeof(Color565) == 2, "565模式下Color应为2字节");

template <class T>
concept ColorTemplate = requires{std::is_same<T, Color565>::value || std::is_same<T, Color666>::value || std::is_same<T, Color888>::value;};
