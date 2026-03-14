#pragma once

#include "decoder.hpp"
#include <esp_aac_dec.h>

class AAC final : public Decoder
{
public:
	static auto init()
	{
		return esp_aac_dec_register();
	}

	static void deinit()
	{
		esp_audio_dec_unregister(ESP_AUDIO_TYPE_AAC);
	}

	AAC(size_t bufferSize = 4096) :
		Decoder(bufferSize)
	{
		esp_aac_dec_open(nullptr, 0, &handle);
	}

	~AAC()
	{
		esp_aac_dec_close(handle);
		close();
	}

	esp_audio_type_t type() override final
	{
		return esp_audio_type_t::ESP_AUDIO_TYPE_AAC;
	}

	bool open(const char* path) override final
	{
		if (!audioIn.open(path))
		{
			ESP_LOGE(TAG, "open %s failed", path);
			return false;
		}
		esp_aac_dec_reset(handle);
		return true;
	}

	void close() override final
	{
		audioIn.close();
		esp_aac_dec_reset(handle);
	}

	void reset() override final
	{
		audioIn.reset();
		esp_aac_dec_reset(handle);
	}

	size_t decode(void* buffer, size_t bufferSize, esp_audio_dec_info_t* info = nullptr) override final
	{
		esp_audio_dec_out_frame_t frameOut{};
		frameOut.buffer = (uint8_t*)buffer;
		frameOut.len = bufferSize;

		esp_audio_dec_info_t infoDefault{};
		if (info == nullptr) info = &infoDefault;

		// ESP_LOGI(TAG, "decode size = %d", audioIn.getPointer()->len);
		auto ret = esp_aac_dec_decode(handle, audioIn.getPointer(), &frameOut, info);
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

private:
	constexpr static auto TAG = "AAC";
};
