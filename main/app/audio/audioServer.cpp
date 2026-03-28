#include "audioServer.hpp"

#include "audio/mp3.hpp"
#include "audio/aac.hpp"

#include "stringCompare.hpp"

#include "task.hpp"

constexpr static auto TAG = "audioServer";

bool AudioServer::autoDeinit = false;

char AudioServer::path[256]{};
Decoder* AudioServer::decoder{};
uint8_t* AudioServer::decoderBuffer{};
size_t AudioServer::decoderBufferThreshold = DecoderBufferThresholdDefault;
ALC* AudioServer::alc{};
uint8_t* AudioServer::alcBuffer{};

GPIO AudioServer::SD{ GPIO::GPIO_NUM::GPIO_NUM_42, GPIO::Mode::GPIO_MODE_DISABLE };
IIS AudioServer::iis{};

bool AudioServer::audioPause = true;
bool AudioServer::serverRunning = false;

Thread AudioServer::decoderThread{};
Thread AudioServer::loaderThread{};
bool AudioServer::decoderPause = false; // thread负责置true，外部负责置false
bool AudioServer::loaderPause = false; // thread负责置true，外部负责置false

AudioServer::PlayList* AudioServer::playListHead = nullptr;
AudioServer::PlayList* AudioServer::playListRandomHead = nullptr;
AudioServer::PlayList* AudioServer::playListNow = nullptr;
bool AudioServer::randomPlay{};

void AudioServer::pause()
{
	audioPause = true;
	SD.setMode(GPIO::Mode::GPIO_MODE_OUTPUT);
	SD = false;
}

void AudioServer::resume()
{
	SD.setMode(GPIO::Mode::GPIO_MODE_DISABLE);
	decoderPause = loaderPause = audioPause = false;
	iis.start();
	loaderThread.resume();
	decoderThread.resume();
}

bool AudioServer::isPaused()
{
	return decoderPause;
}

bool AudioServer::isInited()
{
	return serverRunning;
}

void AudioServer::init()
{
	MP3::init();
	AAC::init();

	assert(decoder == nullptr);
	alc = new ALC{};

	GPIO{ GPIO::GPIO_NUM::GPIO_NUM_41, GPIO::Mode::GPIO_MODE_OUTPUT } = true; // gain
	pause();

	iis = { GPIO_NUM_38, GPIO_NUM_39, GPIO_NUM_40, 44100, i2s_data_bit_width_t::I2S_DATA_BIT_WIDTH_16BIT, i2s_slot_mode_t::I2S_SLOT_MODE_MONO };

	decoderBuffer = new uint8_t[FrameBufferLength];
	alcBuffer = new uint8_t[FrameBufferLength];

	loaderThread = Thread{ loaderMain, "audio loader", nullptr, Task::Priority::High };
	decoderThread = Thread{ decoderMain, "audio decoder", nullptr, Task::Priority::RealTime };
	serverRunning = loaderThread.isRunning() && decoderThread.isRunning();
	if (!serverRunning)
	{
		loaderThread = {};
		decoderThread = {};
		ESP_LOGE(TAG, "server started failed!");
		pause();
		GPIO{ GPIO::GPIO_NUM::GPIO_NUM_41 }.setMode(GPIO::Mode::GPIO_MODE_DISABLE); // gain
		iis = {};

		delete[] alcBuffer;
		alcBuffer = nullptr;

		delete[] decoderBuffer;
		decoderBuffer = nullptr;

		delete alc;
		alc = nullptr;

		delete decoder;
		decoder = nullptr;

		MP3::deinit();
		AAC::deinit();
	}
}

void AudioServer::deinit()
{
	serverRunning = false;
	if (loaderThread.isRunning())
		loaderThread.resume();
	if (decoderThread.isRunning())
		decoderThread.resume();
}

void AudioServer::setAutoDeinit(bool status)
{
	autoDeinit = status;
}

const char* AudioServer::getFilePath()
{
	return path;
}

bool AudioServer::isOpened()
{
	return isInited() && decoder != nullptr && decoder->isOpen();
}

void AudioServer::openFile(const char* path)
{
	ESP_LOGI(TAG, "open %s", path);
	pause();
	while (isOpened() && !isPaused()) vTaskDelay(1);

	auto pathLength = strlen(path);
	if (stringCompare(path + pathLength - 4, 4, ".mp3", 4))
	{
		if (decoder == nullptr || decoder->type() != Decoder::Type::ESP_AUDIO_TYPE_MP3)
		{
			delete decoder;
			decoder = new MP3{};
		}
	}
	else if (stringCompare(path + pathLength - 4, 4, ".aac", 4))
	{
		if (decoder == nullptr || decoder->type() != Decoder::Type::ESP_AUDIO_TYPE_AAC)
		{
			delete decoder;
			decoder = new AAC{};
		}
	}
	else
	{
		ESP_LOGE(TAG, "unsupport audio file %s", path);
		return;
	}

	strcpy(AudioServer::path, path);
	if (!decoder->open(path)) return;

	// load info
	esp_audio_dec_info_t info{};
	decoder->loadBuffer(2048);
	decoder->decode(decoderBuffer, FrameBufferLength, &info);
	decoder->reset();

	ESP_LOGI(TAG, "sample rate = %dHz, bits = %dbit, %d channal, bitRate = %dkbps", info.sample_rate, info.bits_per_sample, info.channel, info.bitrate / 1000);

	if (info.sample_rate == 0)
	{
		ESP_LOGE(TAG, "sample rate == 0!");
		decoder->close();
		return;
	}

	decoderBufferThreshold = DecoderBufferThresholdDefault;

	auto gain = alc->getGain();
	*alc = ALC{ info.sample_rate, info.channel, info.bits_per_sample };
	alc->setGain(gain);

	iis.setSampleRate(info.sample_rate);
	iis.setBitWidth((i2s_data_bit_width_t)info.bits_per_sample);
	iis.setSlotMode(info.channel == 2 ? i2s_slot_mode_t::I2S_SLOT_MODE_STEREO : i2s_slot_mode_t::I2S_SLOT_MODE_MONO);
}

void AudioServer::close()
{
	ESP_LOGI(TAG, "close file");
	AudioServer::path[0] = '\0';
	if (decoder != nullptr)
		decoder->close();
	pause();
}

void AudioServer::setGain(int8_t gain)
{
	alc->setGain(gain);
}

int8_t AudioServer::getGain()
{
	return alc->getGain();
}

AudioServer::PlayList* AudioServer::getPlayList()
{
	return playListHead;
}

AudioServer::PlayList* AudioServer::getPlayListNow()
{
	return playListNow;
}

bool AudioServer::isPlayListEnabled()
{
	return playListNow != nullptr;
}

void AudioServer::enablePlayList()
{
	ESP_LOGI(TAG, "playlist enable");
	playListNow = randomPlay ? playListRandomHead : playListHead;
	if (playListNow) openFile(playListNow->getPath());
}

void AudioServer::disablePlayList()
{
	ESP_LOGI(TAG, "playlist disable");
	playListNow = nullptr;
}

void AudioServer::addPlayList(const char* path, PlayList* insert)
{
	ESP_LOGI(TAG, "add play list %s", path);

	assert(playListHead != nullptr || insert == nullptr); // 确保list存在，或者insert到nullptr

	if (playListHead == nullptr) [[unlikely]]
	{
		// 没有list，此次创建
		playListHead = new PlayList{};
		playListHead->next = nullptr;
		playListHead->last = nullptr;
		playListHead->id = 0;

		playListRandomHead = playListHead;
		playListRandomHead->randomNext = nullptr;
		playListRandomHead->randomLast = nullptr;

		auto buffer = new char[strlen(path) + 1];
		strcpy(buffer, path);
		playListHead->songPath = buffer;
		return;
	}

	auto newList = new PlayList{};
	auto buffer = new char[strlen(path) + 1];
	strcpy(buffer, path);
	newList->songPath = buffer;

	if (insert == nullptr)
	{
		// add to tail
		auto* p = playListHead;
		while (p->next != nullptr) p = p->next;
		newList->id = p->id + 1;
		newList->last = p;
		p->next = newList;
	}
	else
	{
		// normal add
		newList->id = insert->id;
		newList->next = insert;
		newList->last = insert->last;

		newList->next->last = newList;
		if (newList->last != nullptr) newList->last->next = newList;

		if (insert == playListHead) playListHead = newList;
	}

	// 维护insert以后的id
	while (insert != nullptr)
	{
		insert->id++;
		insert = insert->next;
	}

	playListRandomInsert(newList);
}

void AudioServer::removePlayList(PlayList* playList)
{
	assert(playList != nullptr);

	if (playList == playListNow) switchToNextPlayList();
	if (playList == playListNow) close();
	if (playList == playListNow)
	{
		ESP_LOGW(TAG, "can't remove playlist %s, still playing", playList->getPath());
		return;
	}

	if (playList->next != nullptr) playList->next->last = playList->last;
	if (playList->last != nullptr) playList->last->next = playList->next;

	if (playList->randomNext != nullptr) playList->randomNext->randomLast = playList->randomLast;
	if (playList->randomLast != nullptr) playList->randomLast->randomNext = playList->randomNext;

	if (playList == playListHead)
		playListHead = playListHead->next;

	if (playList == playListRandomHead)
		playListRandomHead = playListRandomHead->next;

	// 维护playList以后的id
	for (auto* subList = playList->next;
		subList != nullptr;
		subList = subList->next)
		subList->id--;

	delete[] playList->songPath;
	playList->songPath = nullptr;
	delete playList;
}

void AudioServer::changePlayList(PlayList* playList, const char* path)
{
	assert(playList != nullptr);

	auto* newBuffer = new char[strlen(path) + 1];
	strcpy(newBuffer, path);
	delete[] std::exchange(playList->songPath, newBuffer);

	if (playList == playListNow)
		openFile(playListNow->getPath());
}

void AudioServer::switchToNextPlayList()
{
	if (playListNow == nullptr) return;

	if (randomPlay)
	{
		if (playListNow->randomNext != nullptr)
			playListNow = playListNow->randomNext;
		else
		{
			shufflePlayList();
			playListNow = playListRandomHead;
		}
	}
	else
	{
		if (playListNow->next != nullptr)
			playListNow = playListNow->next;
		else playListNow = playListHead;
	}

	openFile(playListNow->getPath());
}

void AudioServer::switchToLastPlayList()
{
	if (playListNow == nullptr) return;

	if (playListNow->last != nullptr)
		playListNow = playListNow->last;
	else while (playListNow->next != nullptr)
		playListNow = playListNow->next;

	openFile(playListNow->getPath());
}

void AudioServer::switchToPlayList(PlayList* playList)
{
	if (playListNow == nullptr)
	{
		ESP_LOGW(TAG, "playList not started");
		return;
	}

	if (playList == nullptr)
	{
		ESP_LOGE(TAG, "switch to nullptr playlist!");
		return;
	}

	playListNow = playList;
	openFile(playListNow->getPath());
}

AudioServer::PlayList* AudioServer::getPlayListRandom()
{
	return playListRandomHead;
}

bool AudioServer::isRandomPlayEnabled()
{
	return randomPlay;
}

void AudioServer::enableRandomPlay()
{
	randomPlay = true;
	ESP_LOGI(TAG, "random play enabled");
}

void AudioServer::disableRandomPlay()
{
	randomPlay = false;
	ESP_LOGI(TAG, "random play disabled");
}

void AudioServer::shufflePlayList()
{
	auto* p = playListHead;
	while (p->next) p = p->next;
	auto count = p->id + 1; // id from 0

	if (count <= 2) return;

	ESP_LOGI(TAG, "shuffle playlist");


	PlayList** playListArray = new PlayList * [count];
	p = playListRandomHead;
	for (auto i = 0; i < count; i++)
	{
		playListArray[i] = p;
		p = p->randomNext;
	}

	auto swap = [](PlayList*& a, PlayList*& b) {auto t = a; a = b; b = t;};

	PlayList* lastPlay = playListArray[count - 1];
	for (auto i = 0; i < count; i++)
		swap(playListArray[i], playListArray[rand() % count]);
	if (playListArray[0] == lastPlay)
		swap(playListArray[0], playListArray[(rand() % (count - 1)) + 1]); // 重新交换确保不会重复

	// 应用
	playListRandomHead = playListArray[0];
	playListArray[0]->randomLast = nullptr;
	playListArray[0]->randomNext = playListArray[1];
	for (int i = 1; i < count - 1; i++)
	{
		playListArray[i]->randomLast = playListArray[i - 1];
		playListArray[i]->randomNext = playListArray[i + 1];
	}
	playListArray[count - 1]->randomLast = playListArray[count - 2];
	playListArray[count - 1]->randomNext = nullptr;

	delete[] playListArray;
}

void AudioServer::playListRandomInsert(PlayList* newList)
{
	auto* p = newList;
	while (p->next) p = p->next;
	auto count = p->id; // listcount = id + 1; randomListCount = id; addablePlace = id + 1, from 0..count, while count means head
	auto randInsertAfter = rand() % (count + 1);

	if (randInsertAfter == count) // 定义为Head
	{
		newList->randomLast = nullptr;
		newList->randomNext = playListRandomHead;

		newList->randomNext->randomLast = newList;
		playListRandomHead = newList;
		return;
	}

	p = playListRandomHead;
	for (int i = 0; i < randInsertAfter; i++)
		p = p->randomNext;

	newList->randomLast = p;
	newList->randomNext = p->randomNext;
	if (newList->randomNext) newList->randomNext->randomLast = p;
	newList->randomLast->randomNext = newList;
}

void AudioServer::loaderMain(void*)
{
	constexpr static auto TAG = "audio loader";

	ESP_LOGI(TAG, "started");

	loaderPause = true;
	loaderThread.suspend(); // 挂起自己等待唤醒

	while (serverRunning)
	{
		while (!decoder->isOpen() && serverRunning)
			vTaskDelay(1);
		if (!serverRunning) break;

		// load
		while (true)
		{
			if (audioPause) [[unlikely]]
			{
				loaderPause = true;
				loaderThread.suspend();
				if (!serverRunning) break;
			}

			auto loadSize = decoder->getBuffer().tryLoad(DecoderBufferLength); // 这个逻辑应该由信号出发，而不是轮询
			if (loadSize == AudioBuffer::NoNeedToLoad) [[likely]]
				vTaskDelay(1);
			else if (loadSize == 0) [[unlikely]]
			{
				loaderPause = true;
				loaderThread.suspend();
				break;
			}
		}
	}

	ESP_LOGI(TAG, "stoped");
	loaderThread = {};
}

void AudioServer::decoderMain(void*)
{
	constexpr static auto TAG = "audio decoder";

	ESP_LOGI(TAG, "started");

	decoderPause = audioPause = true;
	decoderThread.suspend(); // 挂起自己等待唤醒

	while (serverRunning)
	{
		while (!decoder->isOpen() && serverRunning)
			vTaskDelay(1);
		if (!serverRunning) break;

		// load & decode & output
		while (true)
		{
			if (audioPause)
			{
				if (iis.isInited())
					iis.stop();
				decoderPause = true;
				decoderThread.suspend();
				if (!serverRunning) break;
			}

			while (decoder->getBuffer().getReference().len < decoderBufferThreshold && !loaderPause)
			{
				decoder->getBuffer().update(decoderBufferThreshold);
				vTaskDelay(1);
			}

			auto size = decoder->decode(decoderBuffer, FrameBufferLength);
			if (size == 0) [[unlikely]]
			{
				if (!audioPause && loaderPause) [[unlikely]] break; // 文件结束

				// 检查buffer还有没有增加的空间
				if (decoderBufferThreshold < DecoderBufferThresholdMax) [[likely]]
				{
					decoderBufferThreshold += DecoderBufferThresholdIncrease;
					ESP_LOGI(TAG, "decoderBufferThreshold increased to %d", decoderBufferThreshold);
					continue; // 重新load
				}
				break;
			}

			(*alc)(decoderBuffer, alcBuffer, size); // 音量调节

			auto* pointer = alcBuffer;
			while (!audioPause)
			{
				auto transmitedSize = iis.transmit(pointer, size, 1);
				size -= transmitedSize;
				pointer += transmitedSize;
				if (size == 0) break;
			}
		}

		// finish
		decoder->close();
		pause();
		vTaskDelay(10);
		iis.stop();

		if (playListNow)
		{
			switchToNextPlayList();
			resume();
		}
		else if (autoDeinit) deinit();
	}

	// delete self

	pause();
	GPIO{ GPIO::GPIO_NUM::GPIO_NUM_41 }.setMode(GPIO::Mode::GPIO_MODE_DISABLE); // gain

	iis = {};

	delete[] alcBuffer;
	alcBuffer = nullptr;

	delete alc;
	alc = nullptr;

	delete[] decoderBuffer;
	decoderBuffer = nullptr;

	delete decoder;
	decoder = nullptr;

	MP3::deinit();
	AAC::deinit();

	ESP_LOGI(TAG, "stoped");
	decoderThread = {};
}
