#include "audio.hpp"

#include "audio/mp3.hpp"
#include "audio/iis.hpp"
#include "audio/test.inl"

#include "app/explorer/explorer.hpp"

#include "task.hpp"

EXT_RAM_BSS_ATTR TaskHandle_t audioDeamonHandle{};
EXT_RAM_BSS_ATTR StackType_t* audioDeamonStack{};
EXT_RAM_BSS_ATTR StaticTask_t* audioDeamonTask{}; // must in internal ram

void AppAudio::init()
{
	if (audioDeamonStack != nullptr) return;

	auto* appExplorer = new AppExplorer{ lcd, touch, changeAppCallback, newAppCallback };
	appExplorer->setTitleBuffer(AutoLnaguage{"play audio", "播放音乐"});
	appExplorer->callBackParam = appExplorer;
	appExplorer->openFileCallback = [](const char* path, void* param)
		{
			if (path == nullptr) return;

			auto& self = *(AppExplorer*)param;

			auto length = strlen(path);
			auto pathToDeamon = new char[length + 1]; // one for \0
			strcpy(pathToDeamon, path);

			audioDeamonStack = new StackType_t[4096];

			audioDeamonTask = (StaticTask_t*)heap_caps_malloc(sizeof(StaticTask_t), MALLOC_CAP_INTERNAL);

			audioDeamonHandle = xTaskCreateStatic(deamonMain, "audio deamon", 4096, pathToDeamon, 2, audioDeamonStack, audioDeamonTask);

			self.exit();
		};
	explorer = appExplorer;
}

void AppAudio::touchUpdate()
{
	changeAppCallback(explorer);
}

void AppAudio::deamonMain(void* param)
{
	char* path = (char*)param;
	audioTest(path);
	delete[] path;
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
