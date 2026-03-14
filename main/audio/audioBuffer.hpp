#pragma once

#include <esp_log.h>

#include "storge/fat.hpp"

#include "mutex.hpp"

class AudioBuffer
{
public:
	AudioBuffer(size_t bufferSize = 4096) : bufferCapacity{ bufferSize }, fileBuffer{ new uint8_t[bufferSize] }, readBuffer{ new uint8_t[bufferSize] }, rawBuffer{ new uint8_t[bufferSize] } {}

	~AudioBuffer()
	{
		bufferCapacity = 0;

		delete[] fileBuffer;
		fileBuffer = nullptr;

		delete[] readBuffer;
		readBuffer = nullptr;

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
		readBufferSize = 0;
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
		// fileBufferSize == 0, updateReadBuffer不会发生，操作fileBuffer安全

		auto loadSize = std::min(bufferCapacity, loadSizeMax);
		auto ret = fileBufferSize = audioFile.read(fileBuffer, loadSize);
		updateReadBuffer();
		// ESP_LOGI(TAG, "loaded %d", ret);
		return ret;
	}

	size_t update(size_t targetSize = 0)
	{
		if (targetSize == 0) targetSize = rawIn.consumed;
		if (rawIn.len > targetSize) return NoNeedToLoad; // 无需加载
		assert(targetSize < bufferCapacity);

		updateReadBuffer();
		if (readBufferSize == 0) return 0; // 无法加载

		if (rawIn.len > readBufferStart) // copy加载
		{
			Lock lock{ bufferMutex };
			memcpy(rawBuffer, rawIn.buffer, rawIn.len);
			auto copySize = std::min<size_t>(targetSize - rawIn.len, readBufferSize);
			memcpy(rawBuffer + rawIn.len, readBuffer + readBufferStart, copySize);
			readBufferStart += copySize;
			readBufferSize -= copySize;
			rawIn.len += copySize;
			rawIn.buffer = rawBuffer;

			// ESP_LOGI(TAG, "buffer copyed to %d, coped %d", rawIn.len, copySize);
		}
		else // swap加载
		{
			Lock lock{ bufferMutex };

			readBufferStart -= rawIn.len;
			readBufferSize += rawIn.len;

			// auto copySize = rawIn.len;
			memcpy(readBuffer + readBufferStart, rawIn.buffer, rawIn.len);

			rawIn.buffer = readBuffer + readBufferStart;
			rawIn.len = readBufferSize;
			std::swap(rawBuffer, readBuffer);
			readBufferStart = 0;
			readBufferSize = 0;

			// ESP_LOGI(TAG, "buffer swaped to %d, coped %d", rawIn.len, copySize);
		}
		return rawIn.len;
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
	size_t fileBufferSize{};

	uint8_t* readBuffer = nullptr;
	size_t readBufferStart{};
	size_t readBufferSize{};

	uint8_t* rawBuffer = nullptr;
	esp_audio_dec_in_raw_t rawIn{};

	size_t updateReadBuffer()
	{
		if (readBufferSize == 0 && fileBufferSize != 0)
		{
			// readBufferSize == 0，update不会发生，操作readBuffer安全
			std::swap(fileBuffer, readBuffer);
			readBufferStart = 0;
			readBufferSize = fileBufferSize;
			fileBufferSize = 0;
			return readBufferSize;
		}
		return NoNeedToLoad;
	}
};
