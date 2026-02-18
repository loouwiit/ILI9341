#include "audio.hpp"

#include "app/explorer/explorer.hpp"

#include "task.hpp"

#include "audioServer.hpp"

void AppAudio::init()
{
	auto* appExplorer = new AppExplorer{ lcd, touch, changeAppCallback, newAppCallback };
	appExplorer->setTitleBuffer(AutoLnaguage{ "play audio", "播放音乐" });
	appExplorer->callBackParam = appExplorer;
	appExplorer->openFileCallback = [](const char* path, void* param)
		{
			if (path == nullptr) return;

			auto& self = *(AppExplorer*)param;

			if (!AudioServer::isInited())
				AudioServer::init();
			AudioServer::play(path);

			self.exit();
		};
	explorer = appExplorer;
}

void AppAudio::touchUpdate()
{
	changeAppCallback(explorer);
}
