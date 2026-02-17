#include "audioServer.hpp"

#include "task.hpp"

constexpr static auto TAG = "audioServer";

MP3* AudioServer::mp3Loader{};
uint8_t* AudioServer::frameBuffer{};

TaskHandle_t AudioServer::audioServerHandle{};
StackType_t* AudioServer::audioServerStack{};
StaticTask_t* AudioServer::audioServerTask{}; // must in internal ram

bool AudioServer::isInited()
{
	return audioServerHandle != nullptr;
}

void AudioServer::init()
{
	MP3::init();

	mp3Loader = new MP3{};

	GPIO{ GPIO::GPIO_NUM::GPIO_NUM_41, GPIO::Mode::GPIO_MODE_OUTPUT } = true;

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
		GPIO{ GPIO::GPIO_NUM::GPIO_NUM_41 }.setMode(GPIO::Mode::GPIO_MODE_DISABLE);
		delete mp3Loader;
		mp3Loader = nullptr;
		MP3::deinit();
	}
}

void AudioServer::deinit()
{
	audioServerHandle = nullptr;
}

bool AudioServer::isPlaying()
{
	return isInited() && mp3Loader->isOpen();
}

void AudioServer::play(const char* path)
{
	ESP_LOGI(TAG, "open %s", path);
	mp3Loader->open(path);
}

void AudioServer::serverMain(void*)
{
	constexpr static auto TAG = "audioServer";
	esp_audio_dec_info_t info{};

	vTaskDelay(10); // wait for value set
	// 如果没有这个延迟会在audioServerHandle判定失败，因为此时线程还未写入数值，该线程优先级比changeApp(deamon)高

	ESP_LOGI(TAG, "started");

	while (audioServerHandle != nullptr)
	{
		while (!mp3Loader->isOpen() && audioServerHandle != nullptr)
			vTaskDelay(1);
		if (audioServerHandle == nullptr) break;

		// mp3Loader opened

		// load info
		mp3Loader->load(frameBuffer, FrameBufferLength, &info);
		mp3Loader->reset();

		ESP_LOGI(TAG, "sample rate = %dHz, bits = %dbit, %d channal, bitRate = %dkbps", info.sample_rate, info.bits_per_sample, info.channel, info.bitrate / 1024);

		if (info.sample_rate == 0)
		{
			ESP_LOGE(TAG, "sample rate == 0!");
			mp3Loader->close();
			continue;
		}

		// init iis
		IIS iis{ GPIO_NUM_38, GPIO_NUM_39, GPIO_NUM_40, info.sample_rate, info.channel == 2 ? i2s_data_bit_width_t::I2S_DATA_BIT_WIDTH_32BIT : i2s_data_bit_width_t::I2S_DATA_BIT_WIDTH_16BIT };

		// load & decode & output
		while (true)
		{
			auto size = mp3Loader->load(frameBuffer, FrameBufferLength);
			if (size == 0) break;
			iis.transmit(frameBuffer, size, portMAX_DELAY);
		}
		mp3Loader->close();
	}

	// delete self

	GPIO{ GPIO::GPIO_NUM::GPIO_NUM_41 }.setMode(GPIO::Mode::GPIO_MODE_DISABLE);

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
