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
	contents[3] = &audioGain;
	contents[4] = &audioGainBar;
	contents[5] = &pauseText;
	contents[6] = &endButton;

	title.position = { LCD::ScreenSize.x / 2, 0 };
	title.position.x -= title.computeSize().x / 2;
	title.computeSize();
	title.clickCallbackParam = this;
	title.releaseCallback = [](Finger&, void* param) { auto& self = *(AppAudio*)param; self.back(); };

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
			self.running = false;
			self.newAppCallback(appExplorer);
		};

	if (AudioServer::isInited())
		audioGainBar.setValue((AudioServer::getGain() + 64) * 2);
	else audioGainBar.setValue((DefaultGain + 64) * 2);
	audioGainBar.clickCallbackParam = this;
	audioGainBar.pressCallback = [](Finger& finger, void* param)
		{
			auto& self = *(AppAudio*)param;
			self.gainBarPressTime = xTaskGetTickCount();
		};
	audioGainBar.releaseCallback = [](Finger& finger, void* param)
		{
			auto& self = *(AppAudio*)param;
			auto deltaTime = xTaskGetTickCount() - self.gainBarPressTime;
			if (deltaTime > BarHoldTime) return;

			auto target = finger.position.x;
			if (target > 2 * 64) target = 2 * 64;
			if (target < 0) target = 0;
			self.audioGainBar.setValue(target);
			self.drawLocked = false;
			if (!AudioServer::isInited()) return;
			int8_t gain = self.audioGainBar.getValue() / 2 - 64;
			AudioServer::setGain(gain);
		};
	audioGainBar.holdCallback = [](Finger&, void* param)
		{
			auto& self = *(AppAudio*)param;
			if (self.audioGainBar.getValue() > 2 * 64)
				self.audioGainBar.setValue(2 * 64);
			self.drawLocked = false;
			if (!AudioServer::isInited()) return;
			int8_t gain = self.audioGainBar.getValue() / 2 - 64;
			AudioServer::setGain(gain);
		};

	AudioServer::setAutoDeinit(false);
	audioOpened = true; // 假定上一次状态，从而激活deamon的reload
	// 该任务应该由server完成，此处需要重构
	updatePauseStatus();
	pauseText.position -= pauseText.computeSize() / 2;
	pauseText.computeSize();
	pauseText.clickCallbackParam = this;
	pauseText.releaseCallback = [](Finger&, void* param)
		{
			auto& self = *(AppAudio*)param;
			self.switchPause();
		};

	endButton[0] = &endButtonBackground;
	endButton[1] = &endButtonFrontground;

	endButton.start -= endButton.getSize() / 2;

	endButton.start.x += pauseText.getSize().x; // endButton.end未同步修改
	endButton.start.x += GapSize;

	endButtonBackground.clickCallbackParam = this;
	endButtonBackground.releaseCallback = [](Finger&, void* param)
		{
			auto& self = *(AppAudio*)param;
			self.end();
		};

	ESP_LOGI(TAG, "deamon started");
	deamonRunning = true;
	Task::addTask(deamonTask, "audio", this, 100);

	drawLocked = false;
}

void AppAudio::focusIn()
{
	running = true;
	drawLocked = false;
}

void AppAudio::deinit()
{
	running = false;
	deamonRunning = false;
}

void AppAudio::draw()
{
	while (drawLocked && running) vTaskDelay(1);
	drawLocked = true;
	lcd.clear();
	lcd.draw(contents);
}

void AppAudio::touchUpdate()
{
	Finger finger[2] = { touch[0],touch[1] };

	if (finger[0].state != Finger::State::None)
		contents.finger(finger[0]);
	if (finger[1].state != Finger::State::None)
		contents.finger(finger[1]);
}

void AppAudio::playAudio(const char* path)
{
	if (path == nullptr)
		return;

	{
		Lock lock{ deamonMutex };
		if (!AudioServer::isInited())
		{
			AudioServer::init();
			AudioServer::setGain(audioGainBar.getValue() / 2 - 64);
		}
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
	drawLocked = false;
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
	drawLocked = false;
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

void AppAudio::end()
{
	Lock lock{ deamonMutex };
	audioPathBuffer[0] = '\0';
	audioFileText.text = audioPathBuffer;
	AudioServer::close();
	audioOpened = false;
	drawLocked = false;
}

TickType_t AppAudio::deamonTask(void* param)
{
	auto& self = *(AppAudio*)param;

	if (!self.deamonRunning)
	{
		self.deleteAble = true;
		if (AudioServer::isInited() && !AudioServer::isOpened())
			AudioServer::deinit();
		else AudioServer::setAutoDeinit(true);
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
			{
				AudioServer::init();
				AudioServer::setGain(self.audioGainBar.getValue() / 2 - 64);
			}
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
