#include "audio.hpp"

#include "app/explorer/explorer.hpp"

#include "task.hpp"

#include "audioServer.hpp"

void AppAudio::init()
{
	contents[0] = &title;
	contents[1] = &audioText;
	contents[2] = &audioFileText;
	contents[3] = &pauseText;

	title.position = { LCD::ScreenSize.x / 2, 0 };
	title.position.x -= title.computeSize().x / 2;
	title.computeSize();
	title.clickCallbackParam = this;
	title.releaseCallback = [](Finger&, void* param) { auto& self = *(AppAudio*)param; self.changeAppCallback(nullptr); };

	auto filePointer = AudioServer::getPlayingPath();
	auto now = filePointer;
	while (*now != '\0') now++;
	while (*now != '/' && *now != '\\' && now >= filePointer) now--;
	now++;
	strcpy(audioFileBuffer, now);
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

	pause();
	pauseText.clickCallbackParam = this;
	pauseText.releaseCallback = [](Finger&, void* param)
		{
			auto& self = *(AppAudio*)param;
			self.switchPause();
		};
}

void AppAudio::draw()
{
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

	if (!AudioServer::isInited())
		AudioServer::init();
	AudioServer::play(path);
	resume();

	auto filePointer = AudioServer::getPlayingPath();
	auto now = filePointer;
	while (*now != '\0') now++;
	while (*now != '/' && *now != '\\' && now >= filePointer) now--;
	now++;
	strcpy(audioFileBuffer, now);
	audioFileText.computeSize();
}

void AppAudio::switchPause()
{
	if (AudioServer::isPaused())
		resume();
	else pause();
}

void AppAudio::pause()
{
	AudioServer::turnOff();
	pauseText.text = "|>";
	pauseText.computeSize();
}

void AppAudio::resume()
{
	AudioServer::turnOn();
	pauseText.text = "||";
	pauseText.computeSize();
}
