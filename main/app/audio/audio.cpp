#include "audio.hpp"

#include "app/explorer/explorer.hpp"
#include "playList.hpp"

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
	contents[6] = &playListButton;
	contents[7] = &endButton;

	title.position = { LCD::ScreenSize.x / 2, 0 };
	title.position.x -= title.computeSize().x / 2;
	title.computeSize();
	title.clickCallbackParam = this;
	title.releaseCallback = [](Finger&, void* param) { auto& self = *(AppAudio*)param; self.back(); };

	updatePlayListStatus();
	updatePathText();

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

	audioPlayListPointer = AudioServer::getPlayListNow();

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


	playListButton.offset(-playListButton.getSize() / 2);
	playListButton.offset(-playListButton.getSize().x / 2, 0);
	playListButton.offset(-pauseText.getSize().x / 2, 0);
	playListButton.offset(-GapSize * 2, 0);

	playListButton[0] = &playListButtonBackground;
	playListButton[1] = &playListButtonLines[0];
	playListButton[2] = &playListButtonLines[1];
	playListButton[3] = &playListButtonLines[2];

	playListButtonBackground.clickCallbackParam = this;
	playListButtonBackground.releaseCallback = [](Finger&, void* param)
		{
			auto& self = *(AppAudio*)param;
			ESP_LOGI(TAG, "playListButton");
			auto app = new AppPlayList{ self.lcd, self.touch, self.changeAppCallback, self.newAppCallback };
			self.running = false;
			self.newAppCallback(app);
		};

	endButton[0] = &endButtonBackground;
	endButton[1] = &endButtonFrontground;

	endButton.offset(-endButton.getSize() / 2);
	endButton.offset(endButton.getSize().x / 2, 0);
	endButton.offset(pauseText.getSize().x / 2, 0);
	endButton.offset(GapSize * 2, 0);

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

	updatePlayListStatus();
	updatePathText();
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
		if (AudioServer::isPlayListEnabled())
		{
			AudioServer::disablePlayList();
			updatePlayListStatus();
		}
		AudioServer::openFile(path);
		audioOpened = AudioServer::isOpened();
		if (!audioOpened) return;
		resume();
	}

	updatePathText();
}

void AppAudio::updatePathText()
{
	strcpy(audioPathBuffer, AudioServer::getFilePath());
	audioFileText.text = getBaseName(audioPathBuffer);
	audioFileText.computeSize();
	drawLocked = false;
}

void AppAudio::updatePlayListStatus()
{
	if (AudioServer::isPlayListEnabled())
	{
		audioText.textColor = audioFileText.textColor = LCD::Color::Blue;
		audioText.text = AutoLnaguage{ "playlist:", "播放列表:" };
	}
	else
	{
		audioText.textColor = audioFileText.textColor = LCD::Color::White;
		audioText.text = AutoLnaguage{ "playing:", "当前播放:" };
	}
	audioText.computeSize();
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
	{
		// 检查playlist是否开启以自动加载文件
		if (AudioServer::getPlayList())
		{
			AudioServer::enablePlayList();
			updatePlayListStatus();
			if (AudioServer::isPlayListEnabled())
				resume();
		}
		return;
	}

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
	if (AudioServer::isPlayListEnabled())
	{
		AudioServer::disablePlayList();
		updatePlayListStatus();
	}

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

	if (!AudioServer::isPlayListEnabled())
	{
		Lock lock{ self.deamonMutex };
		auto opened = AudioServer::isOpened();
		if (opened != self.audioOpened) // reopenFile
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
	else // playListEnabled
	{
		if (self.audioPlayListPointer != AudioServer::getPlayListNow())
		{
			self.updatePathText();
			self.audioPlayListPointer = AudioServer::getPlayListNow();
		}
	}

	return 100;
}
