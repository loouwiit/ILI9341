#include "FT6X36.hpp"

#include <algorithm>
#include <esp_task.h>

FT6X36::FT6X36(IIC& iicBus, GPIO reset, GPIO interrupt) :
	iic{ iicBus, address },
	reset{ reset,GPIO::Mode::GPIO_MODE_OUTPUT },
	interrupt{ interrupt, GPIO::Mode::GPIO_MODE_INPUT, GPIO::Pull::GPIO_PULLUP_ONLY }
{}

FT6X36::FT6X36(FT6X36&& move)
{
	std::swap(move.iic, iic);
	std::swap(move.reset, reset);
	std::swap(move.interrupt, interrupt);
}

FT6X36& FT6X36::operator=(FT6X36&& move)
{
	std::swap(move.iic, iic);
	std::swap(move.reset, reset);
	std::swap(move.interrupt, interrupt);
	return *this;
}

FT6X36::~FT6X36()
{
	if (interrupt != GPIO::NC)
		interrupt.setInterrupt(GPIO::Interrupt::GPIO_INTR_DISABLE);
}

void FT6X36::init()
{
	restart();
	interrupt.setInterrupt(GPIO::Interrupt::GPIO_INTR_NEGEDGE,
		[](void* param)
		{
			bool& needUpdate = *(bool*)param;
			needUpdate = true;
		},
		&needUpdate);
}

void FT6X36::restart()
{
	reset = false;
	vTaskDelay(1);
	reset = true;
}

bool FT6X36::detectSelf()
{
	return iic.detect();
}

bool FT6X36::isNeedUpdate()
{
	return needUpdate;
}

void FT6X36::clearNeedUpdate()
{
	needUpdate = false;
}

void FT6X36::update()
{
	clearNeedUpdate();

	unsigned char nowFingerCount = read(0x02) & 0x0F;
	unsigned char readFingerCount = std::max(nowFingerCount, lastFingerCount);

	switch (readFingerCount)
	{
	case 2:
	{
		updateFromAddress(0x03);
		updateFromAddress(0x09);
		break;
	}
	case 1:
	{
		switch (updateFromAddress(0x03))
		{
		case 1: finger[0].state = Finger::State::None; break;
		case 0: finger[1].state = Finger::State::None; break;
		default:
			finger[0].state = Finger::State::None;
			finger[1].state = Finger::State::None;
			break;
		}
		break;
	}
	case 0:
	{
		finger[0].state = Finger::State::None;
		finger[1].state = Finger::State::None;
		break;
	}
	}

	lastFingerCount = nowFingerCount;
}

Finger FT6X36::operator[](unsigned char i)
{
	return finger[i];
}

unsigned char FT6X36::updateFromAddress(uint8_t address)
{
	uint8_t XH = read(address + 0);
	uint8_t XL = read(address + 1);
	uint8_t YH = read(address + 2);
	uint8_t YL = read(address + 3);
	unsigned char id = YH >> 4;
	if (id == 0x0F) return -1;
	finger[id].state = (Finger::State)(XH >> 6);
	finger[id].position.x = ((YH & 0x0F) << 8) + YL;
	finger[id].position.y = 240 - (((XH & 0x0F) << 8) + XL);
	return id;
}

uint8_t FT6X36::read(uint8_t address)
{
	iic.transmit(address);
	uint8_t data = 0x00;
	iic.receive(&data, 1);
	return data;
}
