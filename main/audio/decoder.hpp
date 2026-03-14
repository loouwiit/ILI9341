#pragma once

#include <algorithm>

#include <esp_audio_dec.h>

#include "audioBuffer.hpp"

class Decoder
{
public:
	using Type = esp_audio_type_t;

	virtual Type type() { return ESP_AUDIO_TYPE_UNSUPPORT; }

	virtual bool open(const char* path) = 0;

	virtual void close() = 0;

	virtual void reset() = 0;

	virtual size_t decode(void* buffer, size_t bufferSize, esp_audio_dec_info_t* info = nullptr) = 0;

	virtual size_t operator()(void* buffer, size_t bufferSize, esp_audio_dec_info_t* info = nullptr)
	{
		return decode(buffer, bufferSize, info);
	}

	virtual ~Decoder() {}

	bool isOpen()
	{
		return audioIn.isOpen();
	}

	auto& getBuffer()
	{
		return audioIn;
	}

	void loadBuffer(size_t loadSizeMax = 4096, size_t targetSize = 0)
	{
		if (audioIn.tryLoad(loadSizeMax) != 0)
			audioIn.update(targetSize);
	}

protected:
	Decoder(size_t bufferSize) :
		audioIn{ bufferSize }
	{}

	constexpr static auto TAG = "Decoder";

	AudioBuffer audioIn{};

	using DecoderHandle_t = void*;
	DecoderHandle_t handle = nullptr;
};
