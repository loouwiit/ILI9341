#pragma once

#include <esp_task.h>
#include "mutex.hpp"

class Task
{
public:
	class Priority
	{
	public:
		constexpr static UBaseType_t Deamon = 2;
		constexpr static UBaseType_t Verylow = 3;
		constexpr static UBaseType_t Low = 4;
		constexpr static UBaseType_t Normal = 5;
		constexpr static UBaseType_t High = 6;
		constexpr static UBaseType_t Veryhigh = 7;
		constexpr static UBaseType_t RealTime = 8;
	};

	constexpr static TickType_t infinityTime = portMAX_DELAY;
	constexpr static TickType_t maxSleepTime = 100;

	using Function_t = TickType_t(*)(void* param); // return value is how long the task will sleep

	static void init();
	static void deinit();

	static void daemonMain(void* param);

	static Task* addTask(Function_t function, const char* name, void* param = nullptr, TickType_t callTick = 0);
	static void removeTask(Task* task);

private:
	Task() = default;

	Function_t function = nullptr;
	void* param = nullptr;
	const char* name = nullptr;
	TickType_t nextCallTick = 0;
	Mutex mutex{};

	Task* last = this;
	Task* next = this;

	static Task head;
};
