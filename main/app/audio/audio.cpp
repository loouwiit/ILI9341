#include "audio.hpp"

#include "app/explorer/explorer.hpp"

#include "task.hpp"

#include "audioServer.hpp"

void AppAudio::init()
{
	App::init();

	contents[0] = &title;
	contents[1] = &audioText;
	contents[2] = &audioFileText;
	contents[3] = &pauseText;

	title.position = { LCD::ScreenSize.x / 2, 0 };
	title.position.x -= title.computeSize().x / 2;
	title.computeSize();
	title.clickCallbackParam = this;
	title.releaseCallback = [](Finger&, void* param) { auto& self = *(AppAudio*)param; self.changeAppCallback(nullptr); };

	strcpy(audioPathBuffer, AudioServer::getFilePath());
	char* now = audioPathBuffer;
	while (*now != '\0') now++;
	while (*now != '/' && *now != '\\' && now >= audioPathBuffer) now--;
	now++;
	audioFileText.text = now;
	audioFileText.computeSize();
	audioFileText.clickCallbackParam = audioText.clickCallbackParam = this;
	audioFileText.releaseCallback = audioText.releaseCallback = [](Finger&, void* param)
		{
			auto& self = *(AppAudio*)param;

			auto* appExplorer = new AppExplorer{ self.lcd, self.touch, self.changeAppCallback, self.newAppCallback };
			appExplorer->setTitleBuffer(AutoLnaguage{ "select audio", "选择音乐" });
			appExplorer->callBackParam = param;
			appExplorer->openFileCallback = [](const char* path, void* param)
				{
					auto& self = *(AppAudio*)param;

					if (path != nullptr)
						self.playAudio(path);

					self.changeAppCallback(nullptr); // 退出explorer
				};
			self.newAppCallback(appExplorer);
		};

	AudioServer::setAutoDeinit(false);
	audioOpened = true; // 假定上一次状态，从而激活deamon的reload
	// 该任务应该由server完成，此处需要重构
	updatePauseStatus();
	pauseText.computeSize();
	pauseText.clickCallbackParam = this;
	pauseText.releaseCallback = [](Finger&, void* param)
		{
			auto& self = *(AppAudio*)param;
			self.switchPause();
		};

	ESP_LOGI(TAG, "deamon started");
	Task::addTask(deamonTask, "audio", this, 100);
}

void AppAudio::deinit()
{
	running = false;
}

void AppAudio::draw()
{
	lcd.clear();
	lcd.draw(contents);
}

void AppAudio::touchUpdate()
{
	contents.finger(touch[0]);
	contents.finger(touch[1]);
}

void AppAudio::playAudio(const char* path)
{
	if (path == nullptr)
		return;

	{
		Lock lock{ deamonMutex };
		if (!AudioServer::isInited())
			AudioServer::init();
		AudioServer::openFile(path);
		audioOpened = AudioServer::isOpened();
		if (!audioOpened) return;
		resume();
	}

	strcpy(audioPathBuffer, AudioServer::getFilePath());
	char* now = audioPathBuffer;
	while (*now != '\0') now++;
	while (*now != '/' && *now != '\\' && now >= audioPathBuffer) now--;
	now++;
	audioFileText.text = now;
	audioFileText.computeSize();
}

void AppAudio::updatePauseStatus()
{
	if (AudioServer::isPaused() || !AudioServer::isOpened())
	{
		audioPaused = true;
		pauseText.text = "|>";
	}
	else
	{
		audioPaused = false;
		pauseText.text = "||";
	}
}

void AppAudio::switchPause()
{
	if (!AudioServer::isOpened())
		return;

	if (AudioServer::isPaused())
		resume();
	else pause();
}

void AppAudio::pause()
{
	AudioServer::pause();
	updatePauseStatus();
}

void AppAudio::resume()
{
	AudioServer::resume();
	updatePauseStatus();
}

TickType_t AppAudio::deamonTask(void* param)
{
	auto& self = *(AppAudio*)param;

	if (!self.running)
	{
		self.deleteAble = true;
		AudioServer::setAutoDeinit(true);
		ESP_LOGI(TAG, "deamon stoped");
		return Task::infinityTime;
	}

	auto paused = AudioServer::isPaused();
	if (paused != self.audioPaused)
		self.updatePauseStatus();

	{
		Lock lock{ self.deamonMutex };
		auto opened = AudioServer::isOpened();
		if (opened != self.audioOpened)
		{
			if (!AudioServer::isInited())
				AudioServer::init();
			auto path = AudioServer::getFilePath();
			if (path[0] != '\0')
			{
				AudioServer::openFile(path);
				self.audioOpened = AudioServer::isOpened();
			}
		}
	}

	return 100;
}
