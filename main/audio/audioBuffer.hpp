#pragma once

#include <esp_log.h>

#include "storge/fat.hpp"

class AudioBuffer
{
public:
	AudioBuffer(size_t bufferSize = 4096 + 512) : rawBuffer{ new uint8_t[bufferSize] }, rawBufferLength{ bufferSize } {}

	~AudioBuffer()
	{
		delete[] rawBuffer;
		rawIn.buffer = rawBuffer = nullptr;
		rawBufferLength = 0;
	}

	auto getBufferSize()
	{
		return rawBufferLength;
	}

	bool isOpen()
	{
		return audioFile.isOpen();
	}

	bool open(const char* path)
	{
		if (!audioFile.open(path))
		{
			ESP_LOGE(TAG, "open %s failed", path);
			return false;
		}
		rawIn.len = 0;
		return true;
	}

	void close()
	{
		rawIn.len = 0;
		audioFile.close();
	}

	void seek(FileBase::OffsetType offset, FileBase::OffsetMode mode = FileBase::OffsetMode::Begin)
	{
		audioFile.setOffset(offset, mode);
		rawIn.len = 0;
	}

	void reset()
	{
		seek(0);
	}

	auto* getPointer()
	{
		return &rawIn;
	}

	auto& getReference()
	{
		return rawIn;
	}

	class LoadSizePresets {
	public:
		constexpr static size_t Small = 512;
		constexpr static size_t Medium = 2048;
		constexpr static size_t Large = 4096;

		constexpr static size_t Infinity = -1;
	};

	void tryLoad(size_t loadThreshold, size_t loadMaxSize)
	{
		if (loadThreshold < rawIn.consumed)
			loadThreshold = rawIn.consumed;
		if (rawIn.len > loadThreshold) return;

		if (audioFile.eof()) return;

		auto length = rawIn.len;
		memcpy(rawBuffer, rawIn.buffer, length);

		length += audioFile.read(rawBuffer + length, std::min<size_t>(rawBufferLength - length, loadMaxSize));

		// ESP_LOGI(TAG, "buffer loaded from %d to %d", rawIn.len, length);

		rawIn.buffer = rawBuffer;
		rawIn.len = length;
	}

	void consume(size_t consumed)
	{
		rawIn.len -= consumed;
		rawIn.buffer += consumed;
	}

private:
	constexpr static char TAG[] = "AudioBuffer";

	IFile audioFile{};
	uint8_t* rawBuffer = nullptr;
	size_t rawBufferLength{};
	esp_audio_dec_in_raw_t rawIn{};
};
