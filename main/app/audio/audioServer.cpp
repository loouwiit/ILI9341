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
	loaderThread.resume();
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

void AudioServer::loaderMain(void*)
{
	constexpr static auto TAG = "audio loader";

	loaderPause = audioPause = true;
	loaderThread.suspend(); // 挂起自己等待唤醒

	ESP_LOGI(TAG, "started");

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

	// delete self
	loaderThread = {};
}

void AudioServer::decoderMain(void*)
{
	constexpr static auto TAG = "audio decoder";

	decoderPause = audioPause = true;
	decoderThread.suspend(); // 挂起自己等待唤醒

	ESP_LOGI(TAG, "started");

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
			}

			while (decoder->getBuffer().getReference().len < decoderBufferThreshold && !loaderPause)
			{
				decoder->getBuffer().update(decoderBufferThreshold);
				vTaskDelay(1);
			}

			if (!audioPause && loaderPause) [[unlikely]] break; // 文件结束

			auto size = decoder->decode(decoderBuffer, FrameBufferLength);
			if (size == 0) [[unlikely]]
			{
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
			while (true)
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

		if (autoDeinit) deinit();
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

	decoderThread = {};
}
