#pragma once

#include "gpio.hpp"
#include "iic.hpp"
#include "vector.hpp"

class FT6X36
{
public:
	constexpr static uint16_t address = 0x38;

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

	FT6X36() = default;
	FT6X36(IIC& iicBus, GPIO reset, GPIO interrupt);
	FT6X36(FT6X36&) = delete;
	FT6X36& operator=(FT6X36&) = delete;
	FT6X36(FT6X36&& move);
	FT6X36& operator=(FT6X36&& move);
	~FT6X36();

	void init();

	bool detectSelf();
	bool isNeedUpdate();
	void clearNeedUpdate();

	void update();

	Finger operator[](unsigned char i);

private:
	IICDevice iic; // RTII会处理

	GPIO reset{};
	GPIO interrupt{};

	bool needUpdate = false;

	Finger finger[2]{};

	unsigned char updateFromAddress(uint8_t address);
	uint8_t read(uint8_t address);
};
