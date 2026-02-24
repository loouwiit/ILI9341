#include "audioServer.hpp"

#include "task.hpp"

constexpr static auto TAG = "audioServer";

bool AudioServer::autoDeinit = false;

char AudioServer::path[256]{};
MP3* AudioServer::mp3Loader{};
uint8_t* AudioServer::frameBuffer{};

GPIO AudioServer::SD{ GPIO::GPIO_NUM::GPIO_NUM_42, GPIO::Mode::GPIO_MODE_DISABLE };
IIS AudioServer::iis{};

bool AudioServer::audioPause = true;
bool AudioServer::serverPaused = false;
TaskHandle_t AudioServer::audioServerHandle{};
StackType_t* AudioServer::audioServerStack{};
StaticTask_t* AudioServer::audioServerTask{}; // must in internal ram

void AudioServer::pause()
{
	audioPause = true;
	SD.setMode(GPIO::Mode::GPIO_MODE_OUTPUT);
	SD = false;
}

void AudioServer::resume()
{
	SD.setMode(GPIO::Mode::GPIO_MODE_DISABLE);
	serverPaused = audioPause = false;
	vTaskResume(audioServerHandle);
	iis.start();
}

bool AudioServer::isPaused()
{
	return serverPaused;
}

bool AudioServer::isInited()
{
	return audioServerHandle != nullptr;
}

void AudioServer::init()
{
	MP3::init();

	mp3Loader = new MP3{};

	GPIO{ GPIO::GPIO_NUM::GPIO_NUM_41, GPIO::Mode::GPIO_MODE_OUTPUT } = true; // gain
	pause();

	iis = { GPIO_NUM_38, GPIO_NUM_39, GPIO_NUM_40, 44100, i2s_data_bit_width_t::I2S_DATA_BIT_WIDTH_16BIT, i2s_slot_mode_t::I2S_SLOT_MODE_MONO };

	frameBuffer = new uint8_t[FrameBufferLength];

	audioServerStack = new StackType_t[4096];

	audioServerTask = (StaticTask_t*)heap_caps_malloc(sizeof(StaticTask_t), MALLOC_CAP_INTERNAL);

	audioServerHandle = xTaskCreateStatic(serverMain, "audio server", 4096, nullptr, Task::Priority::RealTime, audioServerStack, audioServerTask);
	if (audioServerHandle == nullptr)
	{
		ESP_LOGE(TAG, "server handle = nullptr!");
		free(audioServerTask);
		delete[] audioServerStack;
		audioServerStack = nullptr;
		delete[] frameBuffer;
		frameBuffer = nullptr;
		pause();
		GPIO{ GPIO::GPIO_NUM::GPIO_NUM_41 }.setMode(GPIO::Mode::GPIO_MODE_DISABLE); // gain
		iis = {};
		delete mp3Loader;
		mp3Loader = nullptr;
		MP3::deinit();
	}
}

void AudioServer::deinit()
{
	vTaskResume(audioServerHandle);
	audioServerHandle = nullptr;
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
	return isInited() && mp3Loader->isOpen();
}

void AudioServer::openFile(const char* path)
{
	ESP_LOGI(TAG, "open %s", path);
	pause();
	while (!isPaused()) vTaskDelay(1);
	strcpy(AudioServer::path, path);
	if (!mp3Loader->open(path)) return;

	// load info
	esp_audio_dec_info_t info{};
	mp3Loader->loadBuffer(0, MP3::LoadSizePresets::Medium);
	mp3Loader->decode(frameBuffer, FrameBufferLength, &info);
	mp3Loader->reset();

	ESP_LOGI(TAG, "sample rate = %dHz, bits = %dbit, %d channal, bitRate = %dkbps", info.sample_rate, info.bits_per_sample, info.channel, info.bitrate / 1000);

	if (info.sample_rate == 0)
	{
		ESP_LOGE(TAG, "sample rate == 0!");
		mp3Loader->close();
	}

	iis.setSampleRate(info.sample_rate);
	iis.setBitWidth((i2s_data_bit_width_t)info.bits_per_sample);
	iis.setSlotMode(info.channel == 2 ? i2s_slot_mode_t::I2S_SLOT_MODE_STEREO : i2s_slot_mode_t::I2S_SLOT_MODE_MONO);
}

void AudioServer::close()
{
	ESP_LOGI(TAG, "close file");
	AudioServer::path[0] = '\0';
	mp3Loader->close();
	pause();
}

void AudioServer::serverMain(void*)
{
	constexpr static auto TAG = "audioServer";

	serverPaused = audioPause = true;
	vTaskSuspend(nullptr); // 挂起自己等待唤醒

	ESP_LOGI(TAG, "started");

	while (audioServerHandle != nullptr)
	{
		while (!mp3Loader->isOpen() && audioServerHandle != nullptr)
			vTaskDelay(1);
		if (audioServerHandle == nullptr) break;

		// load & decode & output
		while (true)
		{
			if (audioPause)
			{
				if (iis.isInited())
					iis.stop();
				serverPaused = true;
				vTaskSuspend(nullptr);
			}

			mp3Loader->loadBuffer();
			auto size = mp3Loader->decode(frameBuffer, FrameBufferLength);
			if (size == 0) break;

			auto* pointer = frameBuffer;
			while (true)
			{
				// ESP_LOGI(TAG, "try transmit %d", size);
				auto transmitedSize = iis.transmit(pointer, size, 0);
				size -= transmitedSize;
				pointer += transmitedSize;
				if (size == 0) break;
				// ESP_LOGI(TAG, "remain %d", size);
				mp3Loader->loadBuffer(mp3Loader->getBuffer().getBufferSize() - MP3::LoadSizePresets::Small, MP3::LoadSizePresets::Small); // 未发送完全，有空闲时间进行加载
			}
		}

		// finish
		mp3Loader->close();
		pause();

		if (autoDeinit) deinit();
	}

	// delete self

	pause();
	GPIO{ GPIO::GPIO_NUM::GPIO_NUM_41 }.setMode(GPIO::Mode::GPIO_MODE_DISABLE); // gain

	iis = {};

	delete[] frameBuffer;
	frameBuffer = nullptr;

	delete mp3Loader;
	mp3Loader = nullptr;

	MP3::deinit();

	Task::addTask([](void*) ->TickType_t
		{
			free(audioServerTask);
			audioServerTask = nullptr;
			delete[] audioServerStack;
			audioServerStack = nullptr;
			ESP_LOGI(TAG, "stoped");
			return Task::infinityTime;
		}, "delete audio server", nullptr, 100);

	vTaskDelete(nullptr);
}
