#include "playList.hpp"

#include "app/explorer/explorer.hpp"

#include "audioServer.hpp"

void AppPlayList::init()
{
	App::init();

	contents[0] = &title;
	contents[1] = &playListModeText;
	contents[2] = &lastSongText;
	contents[3] = &addText;
	contents[4] = &nextSongText;
	contents[5] = &playListLayar;

	title.position = { LCD::ScreenSize.x / 2, 0 };
	title.position.x -= title.computeSize().x / 2;
	title.computeSize();
	title.clickCallbackParam = this;
	title.releaseCallback = [](Finger&, void* param) { auto& self = *(AppPlayList*)param; self.back(); };

	playListModeText.clickCallbackParam = this;
	playListModeText.releaseCallback = [](Finger&, void* param)
		{
			auto& self = *(AppPlayList*)param;
			if (AudioServer::isRandomPlayEnabled())
				AudioServer::disableRandomPlay();
			else AudioServer::enableRandomPlay();
			self.updatePlayListMode();
			self.loadTexts();
		};
	playListModeText.holdCallback = [](Finger&, void* param)
		{
			if (!AudioServer::isRandomPlayEnabled()) return;
			auto& self = *(AppPlayList*)param;
			AudioServer::shufflePlayList();
			self.loadTexts();
		};

	lastSongText.releaseCallback = [](Finger&, void*)
		{
			auto playing = AudioServer::isOpened() && !AudioServer::isPaused();
			AudioServer::switchToLastPlayList();
			if (playing) AudioServer::resume();
		};

	addText.clickCallbackParam = this;
	addText.releaseCallback = [](Finger&, void* param) {
		auto& self = *(AppPlayList*)param;

		{
			Lock lock{ self.addMutex };
			if (!self.running) return; // 已经创建过，避免二次创建app
			self.running = false;
		}

		auto* app = new AppExplorer{ self.lcd, self.touch, self.changeAppCallback, self.newAppCallback };
		app->setTitleBuffer(AutoLnaguage{ "add audio", "添加音乐" });
		app->callBackParam = &self;
		app->openFileCallback = [](const char* path, void* param) {
			auto& self = *(AppPlayList*)param;

			if (path == nullptr)
			{
				// 取消
				self.changeAppCallback(nullptr);
				return;
			}

			auto pathLength = strlen(path);
			if (path[pathLength - 1] != '/')
			{
				// 文件
				AudioServer::addPlayList(path);
				self.changeAppCallback(nullptr); // 退出explorer
				return;
			}

			// 文件夹

			// 存储参数
			while (self.floorAddingParam) vTaskDelay(1);
			self.floorAddingParam = new FloorAddingParam{ self, "", pathLength, true };
			strcpy(self.floorAddingParam->path, path);

			// 由于floor文件数量不定且open速度有可能缓慢，文件夹open将异步执行
			Task::addTask([](void* param)->TickType_t {
				auto* floorAddingParam = (FloorAddingParam*)param;
				auto& [self, path, pathLength, selfCallable] = *floorAddingParam;

				Floor floor{};
				floor.open(path);
				while (true)
				{
					// 加载每一个文件
					auto fileName = floor.read(Floor::Type::File);
					if (fileName == nullptr) break;

					strcpy(path + pathLength, fileName);
					AudioServer::addPlayList(path);
				}
				if (selfCallable) self.loadTexts();
				if (selfCallable) self.floorAddingParam = nullptr;
				delete floorAddingParam;
				return Task::infinityTime;
				}, "playList floor add", self.floorAddingParam, 100, Task::Affinity::None); // 异步加载文件夹

			self.changeAppCallback(nullptr); // 退出explorer
			}; // exploere的openFileCallback

		// 继续创建&启动explorer
		self.newAppCallback(app);
		};
	addText.holdCallback = [](Finger&, void* param)
		{
			auto& self = *(AppPlayList*)param;
			ESP_LOGI(TAG, "clear playlist");
			AudioServer::disablePlayList();
			while (true)
			{
				auto* p = AudioServer::getPlayList();
				if (p) AudioServer::removePlayList(p);
				else break;
			}
			self.drawLocked = false;
			self.loadTexts(); // deamon停止，所以只能手动了
		};
	updatePlayListMode();

	nextSongText.releaseCallback = [](Finger&, void*)
		{
			auto playing = AudioServer::isOpened() && !AudioServer::isPaused();
			AudioServer::switchToNextPlayList(false);
			if (playing) AudioServer::resume();
		};

	for (int i = 0; i < PlayListMaxSize; i++)
	{
		playListLayar[i] = &playListText[i];
		playListText[i].textColor = LCD::Color::White;
		playListText[i].backgroundColor = BackgroundColor;
		playListText[i].scale = TextSize;
		playListText[i].text = "";
		playListText[i].clickCallbackParam = &playListCallbackParam[i];
		playListCallbackParam[i] = this;
	}
	for (int i = 1; i < PlayListMaxSize;i++)
	{
		playListText[i].position.y = playListText[i - 1].position.y + playListText[i - 1].computeSize().y + GapSize;
	}
	playListText[PlayListMaxSize - 1].computeSize();
	loadTexts();

	if (AudioServer::isPlayListEnabled())
	{
		deamonRunning = Task::addTask(deamonTask, "app play list", this, 100) != nullptr;
		if (deamonRunning) ESP_LOGI(TAG, "deamon started");
		else ESP_LOGE(TAG, "deamon start failed");
	}
}

void AppPlayList::focusIn()
{
	running = true;
	loadTexts();
}

void AppPlayList::deinit()
{
	running = false;
	if (floorAddingParam) floorAddingParam->selfCallable = false;
	deleteAble = !deamonRunning;
	deamonRunning = false;
}

void AppPlayList::draw()
{
	while (drawLocked && running) vTaskDelay(1);
	drawLocked = true;
	lcd.clear();
	lcd.draw(contents);
}

void AppPlayList::touchUpdate()
{
	Finger finger[2] = { touch[0],touch[1] };

	if (finger[0].state == Finger::State::Press) do
	{
		fingerActive[0] = true;
		fingerHoldTick[0] = xTaskGetTickCount() + holdTickThreshold;

		lastFingerPosition[0] = finger[0].position;
		fingerMoveTotol[0] = {};
	} while (false);

	if (fingerActive[0]) do
	{
		auto movement = finger[0].position - lastFingerPosition[0];
		if (movement == Vector2s{ 0, 0 }) break;
		fingerMoveTotol[0] += movement;
		contents.start += movement;
		lastFingerPosition[0] = finger[0].position;
		drawLocked = false;
	} while (false);

	if (finger[0].state == Finger::State::Realease) do
	{
		if (abs2(fingerMoveTotol[0]) < moveThreshold2)
		{
			if (xTaskGetTickCount() > fingerHoldTick[0])
				click({ Finger::State::Hold, finger[0].position });
			else click(finger[0]);
		}
		fingerActive[0] = false;
		releaseDetect();
	} while (false);

	if (finger[1].state == Finger::State::Press) do
	{
		fingerActive[1] = true;
		fingerHoldTick[1] = xTaskGetTickCount() + holdTickThreshold;

		lastFingerPosition[1] = finger[1].position;
		fingerMoveTotol[1] = {};
	} while (false);

	if (fingerActive[1]) do
	{
		auto movement = finger[1].position - lastFingerPosition[1];
		if (movement == Vector2s{ 0, 0 }) break;
		fingerMoveTotol[1] += movement;
		contents.start += movement;
		lastFingerPosition[1] = finger[1].position;
		drawLocked = false;
	} while (false);

	if (finger[1].state == Finger::State::Realease) do
	{
		if (abs2(fingerMoveTotol[1]) < moveThreshold2)
		{
			if (xTaskGetTickCount() > fingerHoldTick[1])
				click({ Finger::State::Hold, finger[1].position });
			else click(finger[1]);
		}
		fingerActive[1] = false;
		releaseDetect();
	} while (false);
}

void AppPlayList::updatePlayListMode()
{
	if (AudioServer::isRandomPlayEnabled())
		playListModeText.text = AutoLnaguage{ "random play","随机播放" };
	else
		playListModeText.text = AutoLnaguage{ "sequential play","顺序播放" };

	lastSongText.position.x = playListModeText.position.x + playListModeText.computeSize().x + GapSize;
	addText.position.x = lastSongText.position.x + lastSongText.computeSize().x + GapSize;
	nextSongText.position.x = addText.position.x + addText.computeSize().x + GapSize;
	nextSongText.computeSize();
	drawLocked = false;
}

void AppPlayList::loadTexts()
{
	auto* playListNow = AudioServer::getPlayListNow();
	auto* p = AudioServer::isRandomPlayEnabled() ? AudioServer::getPlayListRandom() : AudioServer::getPlayList();
	playListLayar.elementCount = 0;

	if (p != nullptr) do
	{
		auto& i = playListLayar.elementCount;
		playListText[i].textColor = p == playListNow ? LCD::Color::Blue : LCD::Color::White;
		playListText[i].text = getBaseName(p->getPath());
		playListText[i].fonts = &fontsDefault;
		playListText[i].computeSize();
		playListText[i].holdCallback = [](Finger&, void* param)
			{
				auto& self = **(AppPlayList**)param;
				unsigned id = (AppPlayList**)param - self.playListCallbackParam;

				// 寻找id
				auto* p = AudioServer::isRandomPlayEnabled() ? AudioServer::getPlayListRandom() : AudioServer::getPlayList();
				if (AudioServer::isRandomPlayEnabled())
					for (unsigned i = 0; i < id; i++) p = p->getRandomNext();
				else
					for (unsigned i = 0; i < id; i++) p = p->getNext();

				if (p == nullptr) [[unlikely]]
				{
					ESP_LOGE(TAG, "hold to play list with nullptr! id = %d", id);
					return;
				}

				// 其他音乐：删除音乐
				if (p != AudioServer::getPlayListNow())
				{
					ESP_LOGI(TAG, "remove play list %d: %s", id, p->getPath());
					AudioServer::removePlayList(p);
					self.loadTexts(); // 这个必须手动更新
					return;
				}

				// 当前音乐：修改音乐
				ESP_LOGI(TAG, "change %d: %s", id, p->getPath());

				auto* app = new AppExplorer{ self.lcd, self.touch, self.changeAppCallback, self.newAppCallback };
				app->setTitleBuffer(AutoLnaguage{ "change audio", "修改音乐" });
				app->callBackParam = param;
				app->openFileCallback = [](const char* path, void* param)
					{
						auto& self = **(AppPlayList**)param;
						unsigned id = (AppPlayList**)param - self.playListCallbackParam;

						if (path == nullptr)
						{
							self.changeAppCallback(nullptr);
							return;
						}

						if (path[strlen(path) - 1] == '/') return; // 文件夹

						auto* p = AudioServer::getPlayList(); // 寻找id
						while (p && p->getId() != id) p = p->getNext();

						// 对p修改音乐
						if (p)
						{
							auto isThisSongPlaying = AudioServer::getPlayListNow() == p && AudioServer::isOpened() && !AudioServer::isPaused();
							AudioServer::changePlayList(p, path);
							if (isThisSongPlaying) AudioServer::resume();
						}

						// 退出explorer
						self.changeAppCallback(nullptr);
					};
				self.newAppCallback(app);
				self.running = false;
			};
		playListText[i].releaseCallback = [](Finger&, void* param)
			{
				auto& self = **(AppPlayList**)param;
				unsigned id = (AppPlayList**)param - self.playListCallbackParam;

				// 寻找id
				auto* p = AudioServer::isRandomPlayEnabled() ? AudioServer::getPlayListRandom() : AudioServer::getPlayList();
				if (AudioServer::isRandomPlayEnabled())
					for (unsigned i = 0; i < id; i++) p = p->getRandomNext();
				else
					for (unsigned i = 0; i < id; i++) p = p->getNext();

				// 未激活 -> 激活playlist
				if (!AudioServer::isPlayListEnabled()) [[unlikely]]
				{
					AudioServer::enablePlayList();
					AudioServer::switchToPlayList(p);
					self.audioPlayListPointer = nullptr; // 激活deamon
					self.deamonRunning = Task::addTask(deamonTask, "app play list", &self, 100) != nullptr;
					if (self.deamonRunning) ESP_LOGI(TAG, "deamon started");
					else ESP_LOGE(TAG, "deamon start failed");
					return;
				}

				if (p == nullptr) [[unlikely]]
				{
					ESP_LOGE(TAG, "switch play list with nullptr! id = %d", id);
					return;
				}

				// 当前音乐：切换暂停
				if (p == AudioServer::getPlayListNow())
				{
					ESP_LOGI(TAG, "switch pause status %d: %s", id, p->getPath());
					if (AudioServer::isPaused())
						AudioServer::resume();
					else AudioServer::pause();
					return;
				}

				// 其他音乐：切换音乐
				ESP_LOGI(TAG, "switch play list %d: %s", id, p->getPath());
				auto playing = AudioServer::isOpened() && !AudioServer::isPaused(); // 继续播放
				AudioServer::switchToPlayList(p);
				if (playing) AudioServer::resume();
			};
		i++;
		p = AudioServer::isRandomPlayEnabled() ? p->getRandomNext() : p->getNext();
		if (i >= PlayListMaxSize) break;
	} while (p != nullptr);

	drawLocked = false;
}

void AppPlayList::click(Finger finger)
{
	contents.finger(finger);
}

void AppPlayList::releaseDetect()
{
	if (touch[0].state != Finger::State::Contact && touch[1].state != Finger::State::Contact)
	{
		if (contents.start.y > 0)
		{
			contents.start.y = 0;
			drawLocked = false;
		}
		if (contents.start.x > 0)
		{
			contents.start.x = 0;
			drawLocked = false;
		}
	}
}

TickType_t AppPlayList::deamonTask(void* param)
{
	auto& self = *(AppPlayList*)param;

	if (!self.deamonRunning || !AudioServer::isPlayListEnabled())
	{
		self.deamonRunning = false;
		self.deleteAble = true;
		if (AudioServer::isInited() && !AudioServer::isOpened())
			AudioServer::deinit();
		else AudioServer::setAutoDeinit(true);
		ESP_LOGI(TAG, "deamon stoped");
		return Task::infinityTime;
	}

	if (self.audioPlayListPointer != AudioServer::getPlayListNow())
	{
		self.audioPlayListPointer = AudioServer::getPlayListNow();
		self.loadTexts();
	}

	return 100;
}
