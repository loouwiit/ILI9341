#pragma once

#include <esp_log.h>

#include "storge/fat.hpp"

#include "mutex.hpp"

class AudioBuffer
{
public:
	AudioBuffer(size_t bufferSize = 4096) : bufferCapacity{ bufferSize }, fileBuffer{ new uint8_t[bufferSize] }, rawBuffer{ new uint8_t[bufferSize] } {}

	~AudioBuffer()
	{
		bufferCapacity = 0;

		delete[] fileBuffer;
		fileBuffer = nullptr;

		delete[] rawBuffer;
		rawIn.buffer = rawBuffer = nullptr;
	}

	auto getBufferSize()
	{
		return bufferCapacity;
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
		fileBufferSize = 0;
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

	constexpr static size_t NoNeedToLoad = -1;
	size_t tryLoad(size_t loadSizeMax = 4096)
	{
		if (fileBufferSize != 0) return NoNeedToLoad;
		// fileBufferSize == 0, update不会发生，操作fileBuffer安全
		fileBufferStart = 0;
		return fileBufferSize = audioFile.read(fileBuffer, std::min(bufferCapacity, loadSizeMax));
	}

	void update()
	{
		if (rawIn.len > rawIn.consumed) return; // 无需加载

		if (fileBufferSize == 0) return; // 无法加载

		if (rawIn.len > fileBufferStart) // copy加载
		{
			Lock lock{ bufferMutex };
			memcpy(rawBuffer, rawIn.buffer, rawIn.len);
			auto copySize = rawIn.consumed - rawIn.len;
			memcpy(rawBuffer + rawIn.len, fileBuffer, copySize);
			fileBufferStart += copySize;
			fileBufferSize -= copySize;
			rawIn.len += copySize;
			rawIn.buffer = rawBuffer;

			ESP_LOGI(TAG, "buffer copyed to %d", rawIn.len);
		}
		else // swap加载
		{
			Lock lock{ bufferMutex };

			fileBufferStart -= rawIn.len;
			fileBufferSize += rawIn.len;

			memcpy(fileBuffer + fileBufferStart, rawIn.buffer, rawIn.len);

			rawIn.buffer = fileBuffer + fileBufferStart;
			rawIn.len = fileBufferSize;
			std::swap(rawBuffer, fileBuffer);
			fileBufferStart = 0;
			fileBufferSize = 0;

			ESP_LOGI(TAG, "buffer swaped to %d", rawIn.len);
		}
	}

	void consume(size_t consumed)
	{
		bufferMutex.lock();
		rawIn.len -= consumed;
		rawIn.buffer += consumed;
		bufferMutex.unlock();

		update();
	}

private:
	constexpr static char TAG[] = "AudioBuffer";

	IFile audioFile{};
	size_t bufferCapacity{};

	Mutex bufferMutex{};

	uint8_t* fileBuffer = nullptr;
	size_t fileBufferStart{};
	size_t fileBufferSize{};

	uint8_t* rawBuffer = nullptr;
	esp_audio_dec_in_raw_t rawIn{};
};
