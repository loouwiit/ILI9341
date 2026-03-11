#include "task.hpp"
#include <esp_log.h>

Task Task::head{};

EXT_RAM_BSS_ATTR Thread daemonThread{};
static bool daemonRunning = false;

void Task::init()
{
	if (daemonRunning) return;
	daemonRunning = true;
	daemonThread = Thread{ daemonMain, "taskDaemon", nullptr, Task::Priority::Deamon, 4096 };
	if (!daemonThread.isRunning())
		daemonRunning = false;
}

// void Task::deinit()
// {
// 	daemonRunning = false;
// 	for (Task* nowTask = head.next->next; nowTask->last != &head; nowTask = nowTask->next)
// 		removeTask(nowTask->last);
// 	// bug here, who release the stack?
// }

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

Thread::Thread(Function_t function, const char* name, void* param, Priority priority, size_t stackSize)
{
	data.stack = new StackType_t[stackSize];
	data.task = (StaticTask_t*)heap_caps_malloc(sizeof(StaticTask_t), MALLOC_CAP_INTERNAL);
	data.handle = xTaskCreateStatic(function, name, stackSize, param, priority, data.stack, data.task);
	if (data.handle == nullptr)
	{
		ESP_LOGE(TAG, "handle = nullptr! task creat failed");
		free(data.task);
		data.task = nullptr;
		delete[] data.stack;
		data.stack = nullptr;
	}
}

Thread::~Thread()
{
	if (data.handle == nullptr) return;

	ThreadData* saveCopy = new ThreadData;
	*saveCopy = data;

	Task::addTask([](void* param) ->TickType_t
		{
			auto& self = *(ThreadData*)param;
			self.handle = nullptr;
			free(self.task);
			self.task = nullptr;
			delete[] self.stack;
			self.stack = nullptr;
			delete& self;
			return Task::infinityTime;
		}, "delete thread", saveCopy, 100);
	vTaskDelete(data.handle);
}

bool Thread::isRunning()
{
	return data.handle != nullptr;
}

void Thread::suspend()
{
	vTaskSuspend(data.handle);
}

void Thread::resume()
{
	vTaskResume(data.handle);
}
