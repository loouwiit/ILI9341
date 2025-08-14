#pragma once

#include <cstdint>

class Color666
{
public:
	uint8_t R : 6;
	uint8_t : 0;
	uint8_t G : 6;
	uint8_t : 0;
	uint8_t B : 6;

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
	uint8_t R : 5;
	uint8_t GH : 3;
	uint8_t GL : 3;
	uint8_t B : 5;

	const static Color565 White;
	const static Color565 Black;
	const static Color565 Red;
	const static Color565 Green;
	const static Color565 Blue;
};
static_assert(sizeof(Color565) == 2, "565模式下Color应为2字节");
