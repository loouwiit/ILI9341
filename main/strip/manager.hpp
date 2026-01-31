#pragma once

#include "task.hpp"

#include "strip/strip.hpp"
#include "strip/snapshot.hpp"

class Strip::Manager
{
public:
	Manager(Strip& strip) : strip{ strip } {}
	~Manager() { stop(); deinit(); }

	bool isInited();
	void init();
	void deinit();

	bool isStarted();
	void start();
	void stop();

	int getId();
	TickType_t getLastTime();
	void setLastTime(TickType_t lastTime);

	RGB& operator[](uint32_t index);
	void apply();

	void last();
	void next();
	void add();
	void remove();

private:
	constexpr static TickType_t MaxStripTaskSleepTime = 1000;

	Strip& strip;

	Task* stripTask = nullptr;
	Strip::Snapshot* snapshot = nullptr;
	TickType_t snapshotNextChangeTime = Task::infinityTime;

	static TickType_t stripThreadMain(void* param);
};
