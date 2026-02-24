#pragma once

#include <algorithm>

#include <esp_mp3_dec.h>
#include <esp_audio_dec.h>

#include "audioBuffer.hpp"

class MP3
{
public:
	static auto init()
	{
		return esp_mp3_dec_register();
	}

	static void deinit()
	{
		esp_audio_dec_unregister(ESP_AUDIO_TYPE_MP3);
	}

	MP3()
	{
		esp_mp3_dec_open(nullptr, 0, &handle);
	}

	~MP3()
	{
		esp_mp3_dec_close(handle);
		close();
	}

	bool isOpen()
	{
		return audioIn.isOpen();
	}

	bool open(const char* path)
	{
		if (!audioIn.open(path))
		{
			ESP_LOGE(TAG, "open %s failed", path);
			return false;
		}
		esp_mp3_dec_reset(handle);
		return true;
	}

	void close()
	{
		audioIn.close();
		esp_mp3_dec_reset(handle);
	}

	void reset()
	{
		audioIn.reset();
		esp_mp3_dec_reset(handle);
	}

	void loadBuffer(AudioBuffer::LoadStrategy strategy = AudioBuffer::LoadStrategy::Adaptive)
	{
		audioIn.tryLoad(strategy);
	}

	size_t decode(void* buffer, size_t bufferSize, esp_audio_dec_info_t* info = nullptr)
	{
		esp_audio_dec_out_frame_t frameOut{};
		frameOut.buffer = (uint8_t*)buffer;
		frameOut.len = bufferSize;

		esp_audio_dec_info_t infoDefault{};
		if (info == nullptr) info = &infoDefault;

		auto ret = esp_mp3_dec_decode(handle, audioIn.getPointer(), &frameOut, info);
		if (ret != ESP_AUDIO_ERR_OK)
		{
			if (ret == ESP_AUDIO_ERR_BUFF_NOT_ENOUGH)
				ESP_LOGE(TAG, "decode need %d while has %d", frameOut.needed_size, frameOut.len);
			else ESP_LOGE(TAG, "error %d", ret);
		}

		// ESP_LOGI(TAG, "decode %d from %d", frameOut.decoded_size, rawIn.consumed);
		audioIn.consume(audioIn.getReference().consumed);

		return frameOut.decoded_size;
	}

	auto operator()(void* buffer, size_t bufferSize, esp_audio_dec_info_t* info = nullptr)
	{
		return decode(buffer, bufferSize, info);
	}

private:
	constexpr static auto TAG = "MP3";

	AudioBuffer audioIn{};

	using Mp3Handle_t = void*;
	Mp3Handle_t handle = nullptr;
};
