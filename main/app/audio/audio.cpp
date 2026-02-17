#include "audio.hpp"

#include "audio/mp3.hpp"
#include "audio/iis.hpp"
#include "audio/test.inl"

#include "task.hpp"

EXT_RAM_BSS_ATTR TaskHandle_t audioDeamonHandle{};
EXT_RAM_BSS_ATTR StackType_t* audioDeamonStack{};
EXT_RAM_BSS_ATTR StaticTask_t* audioDeamonTask{}; // must in internal ram

void AppAudio::init()
{
	if (audioDeamonStack == nullptr)
	{
		audioDeamonStack = new StackType_t[4096];

		audioDeamonTask = (StaticTask_t*)heap_caps_malloc(sizeof(StaticTask_t), MALLOC_CAP_INTERNAL);

		audioDeamonHandle = xTaskCreateStatic(deamonMain, "audio deamon", 4096, nullptr, 2, audioDeamonStack, audioDeamonTask);
	}
}

void AppAudio::deamonMain(void*)
{
	audioTest("music/16b-2c-48000hz.mp3");
	Task::addTask([](void*) ->TickType_t
		{
			free(audioDeamonTask);
			audioDeamonTask = nullptr;
			delete[] audioDeamonStack;
			audioDeamonStack = nullptr;
			return Task::infinityTime;
		}, "delete audio task", nullptr, 100);
	audioDeamonHandle = nullptr;
	vTaskDelete(nullptr);
}
