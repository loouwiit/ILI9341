#pragma once

#include "vector.hpp"

class Finger {
public:
	enum class State : unsigned char
	{
		Press = 0x00,
		Realease = 0x01,
		Contact = 0x02,
		None = 0x03,

		Hold = Contact,
	};

	State state = State::None;
	Vector2s position{};
};
