#pragma once

#include "strip/strip.hpp"

class Strip::Snapshot
{
public:
	Snapshot() = default;
	Snapshot(Snapshot&& move) { operator=(std::move(move)); }
	Snapshot(Snapshot& copy) { operator=(copy); }
	Snapshot& operator=(Snapshot&& move)
	{
		for (uint32_t i = 0; i < AppStrip::LedCount; i++)
			std::swap(move.color[i], color[i]);

		std::swap(move.lastTime, lastTime);
		std::swap(move.id, id);
		return *this;
	}
	Snapshot& operator=(Snapshot& copy)
	{
		for (uint32_t i = 0; i < AppStrip::LedCount; i++)
			color[i] = copy.color[i];

		lastTime = copy.lastTime;
		id = copy.id;
		return *this;
	}

	Strip::RGB color[AppStrip::LedCount]{};
	TickType_t lastTime = 1000;

	int id = 0;
	Snapshot* next = this;
	Snapshot* last = this;
};