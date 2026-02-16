#include "mp3.hpp"
#include "iis.hpp"

# warning audioTest here

void audioTest(const char* path)
{
	constexpr auto TAG = "audio test";

	MP3::init();
	MP3 mp3Loader{};
	if (!mp3Loader.open(path)) return;

	constexpr size_t FrameBufferLength = 8192;
	auto* frameBuffer = new uint8_t[FrameBufferLength];

	GPIO{ GPIO::GPIO_NUM::GPIO_NUM_41 } = true;
	IIS iis{ GPIO_NUM_38, GPIO_NUM_39, GPIO_NUM_40, 44100 };

	while (true)
	{
		auto size = mp3Loader(frameBuffer, FrameBufferLength);

		if (size == 0) break;

		auto transmitSize = iis.transmit(frameBuffer, size, portMAX_DELAY);
		ESP_LOGI(TAG, "transmited %d", transmitSize);
	}

	delete[] frameBuffer;

	MP3::deinit();
}
