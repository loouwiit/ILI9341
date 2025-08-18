#pragma once

#include "vector.hpp"

class Finger {
public:
	enum class State : unsigned char
	{
		Press = 0x01,
		Realease = 0x02,
		Contact = 0x03,
		None = 0x04,
	};

	State state = State::None;
	Vector2us position{};
};
