#pragma once

#include <esp_mp3_dec.h>
#include <esp_audio_dec.h>

#include "storge/fat.hpp"

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
		rawIn.buffer = new uint8_t[RawBufferLength];
		rawIn.frame_recover = esp_audio_dec_recovery_t::ESP_AUDIO_DEC_RECOVERY_NONE;
		esp_mp3_dec_open(nullptr, 0, &handle);
	}

	~MP3()
	{
		esp_mp3_dec_close(handle);
		delete[] rawIn.buffer;
		rawIn.buffer = nullptr;
		close();
	}

	bool open(const char* path)
	{
		if (!audioFile.open(path))
		{
			ESP_LOGE(TAG, "open %s failed", path);
			return false;
		}
		esp_mp3_dec_reset(handle);
		return true;
	}

	void close()
	{
		esp_mp3_dec_reset(handle);
		audioFile.close();
	}

	void reset()
	{
		audioFile.setOffset(0, FileBase::OffsetMode::Begin);
		esp_mp3_dec_reset(handle);
	}

	size_t load(void* buffer, size_t bufferSize, esp_audio_dec_info_t* info = nullptr)
	{
		rawIn.len = audioFile.read(rawIn.buffer, RawBufferLength);
		audioFile.setOffset(-rawIn.len, FileBase::OffsetMode::Current);

		esp_audio_dec_out_frame_t frameOut{};
		frameOut.buffer = (uint8_t*)buffer;
		frameOut.len = bufferSize;

		esp_audio_dec_info_t infoDefault{};
		if (info == nullptr) info = &infoDefault;

		auto ret = esp_mp3_dec_decode(handle, &rawIn, &frameOut, info);
		if (ret != ESP_AUDIO_ERR_OK)
		{
			if (ret == ESP_AUDIO_ERR_BUFF_NOT_ENOUGH)
				ESP_LOGE(TAG, "decode need %d while has %d", frameOut.needed_size, frameOut.len);
			else ESP_LOGE(TAG, "error %d", ret);
		}

		// ESP_LOGI(TAG, "decode %d from %d", frameOut.decoded_size, rawIn.consumed);
		audioFile.setOffset(rawIn.consumed, FileBase::OffsetMode::Current);

		return frameOut.decoded_size;
	}

	auto operator()(void* buffer, size_t bufferSize, esp_audio_dec_info_t* info = nullptr)
	{
		return load(buffer, bufferSize, info);
	}

private:
	constexpr static auto TAG = "MP3";

	IFile audioFile{};

	constexpr static size_t RawBufferLength = 1536;
	esp_audio_dec_in_raw_t rawIn{};

	using Mp3Handle_t = void*;
	Mp3Handle_t handle = nullptr;
};
