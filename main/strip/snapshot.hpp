#pragma once

#include "strip/strip.hpp"
#include "storge/fat.hpp"

class Strip::Snapshot
{
public:
	~Snapshot() { delete[] color; color = nullptr; };
	Snapshot() = default;
	Snapshot(uint32_t ledCount) : ledCount{ ledCount }, color{ new RGB[ledCount] } {};
	Snapshot(Snapshot&& move) { operator=(std::move(move)); }
	Snapshot(Snapshot& copy) { operator=(copy); }
	Snapshot& operator=(Snapshot&& move)
	{
		std::swap(move.color, color);
		std::swap(move.id, id);
		std::swap(move.ledCount, ledCount);
		std::swap(move.lastTime, lastTime);
		return *this;
	}
	Snapshot& operator=(Snapshot& copy)
	{
		if (ledCount < copy.ledCount)
		{
			delete[] color;
			color = new RGB[copy.ledCount];
		}

		for (uint32_t i = 0; i < copy.ledCount; i++)
			color[i] = copy.color[i];

		id = copy.id;
		ledCount = copy.ledCount;
		lastTime = copy.lastTime;
		return *this;
	}

	friend Strip& operator>>(Strip& strip, Snapshot& snapshot)
	{
		if (snapshot.ledCount < strip.getCount())
		{
			delete[] snapshot.color;
			snapshot.color = new RGB[strip.getCount()];
		}
		snapshot.ledCount = strip.getCount();

		for (uint32_t i = 0; i < snapshot.ledCount; i++)
			snapshot.color[i] = strip[i];

		return strip;
	}

	friend Strip& operator<<(Strip& strip, Snapshot& snapshot)
	{
		uint32_t count = std::min(strip.getCount(), snapshot.ledCount);
		for (uint32_t i = 0; i < count; i++)
			strip[i] = snapshot.color[i];

		return strip;
	}

	void read(IFile& file)
	{
		file.read(color, sizeof(color[0]) * ledCount);
		file.read(&lastTime, sizeof(lastTime));
	}

	void write(OFile& file)
	{
		file.write(color, sizeof(color[0]) * ledCount);
		file.write(&lastTime, sizeof(lastTime));
	}

	friend IFile& operator>>(IFile& file, Snapshot& snapshot)
	{
		snapshot.read(file);
		return file;
	}

	friend OFile& operator<<(OFile& file, Snapshot& snapshot)
	{
		snapshot.write(file);
		return file;
	}

	RGB& operator[](uint32_t index)
	{
		return color[index];
	}

	int id = 0;
	uint32_t ledCount = 0;
	TickType_t lastTime = 1000;

	Snapshot* next = this;
	Snapshot* last = this;

private:
	RGB* color = nullptr;
};