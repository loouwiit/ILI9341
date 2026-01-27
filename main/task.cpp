#include "task.hpp"
#include <esp_log.h>

Task Task::head{};

EXT_RAM_BSS_ATTR static TaskHandle_t daemonHandle{};
EXT_RAM_BSS_ATTR static StackType_t* daemonStack{};
static StaticTask_t daemonTasks{};
static bool daemonRunning = false;

constexpr char TAG[] = "task";

void Task::init()
{
	daemonRunning = true;

	if (daemonStack == nullptr)
		daemonStack = new StackType_t[4096];

	daemonHandle = xTaskCreateStatic(daemonMain, "taskDaemon", 4096, nullptr, 2, daemonStack, &daemonTasks);
	if (daemonHandle == nullptr)
	{
		ESP_LOGE(TAG, "init failed");
		delete[] daemonStack;
		daemonStack = nullptr;
	}
}

void Task::deinit()
{
	daemonRunning = false;
	for (Task* nowTask = head.next->next; nowTask->last != &head; nowTask = nowTask->next)
		removeTask(nowTask->last);
}

void Task::daemonMain(void* param)
{
	while (daemonRunning)
	{
		TickType_t closestTime = infinityTime;
		TickType_t nowTime = xTaskGetTickCount();
		for (Task* nowTask = head.next; nowTask != &head; nowTask = nowTask->next)
		{
			if (nowTask->nextCallTick < nowTime && nowTask->mutex.try_lock())
			{
				TickType_t intervalTick = nowTask->function(nowTask->param);
				if (intervalTick == infinityTime)
				{
					nowTask->nextCallTick = infinityTime;
					nowTask->mutex.unlock();
					removeTask(nowTask);
					continue;
				}
				nowTask->nextCallTick = nowTime + intervalTick;
				nowTask->mutex.unlock();
			}
			if (nowTask->nextCallTick < closestTime)
				closestTime = nowTask->nextCallTick;
		}

		nowTime = xTaskGetTickCount(); // 函数调用可能导致时间流逝，在此处重新获取
		if (closestTime < nowTime)
			closestTime = nowTime + 1; // 至少睡1tick
		auto sleepTime = closestTime - nowTime;
		if (sleepTime > maxSleepTime)
			sleepTime = maxSleepTime; // 最多睡maxSleepTime
		vTaskDelay(sleepTime);
	}
}

Task* Task::addTask(Function_t function, const char* name, void* param, TickType_t firstCallTick)
{
	if (firstCallTick == infinityTime) return nullptr;

	Task* task = new Task{};

	Lock lock{ task->mutex };

	task->function = function;
	task->param = param;
	task->name = name;
	task->nextCallTick = xTaskGetTickCount() + firstCallTick;

	task->last = head.last;
	task->next = &head;

	task->next->last = task;
	task->last->next = task;

	return task;
}

void Task::removeTask(Task* task)
{
	task->mutex.lock();
	task->nextCallTick = infinityTime;
	task->last->next = task->next;
	task->next->last = task->last;
	task->mutex.unlock();

	delete task;
}
