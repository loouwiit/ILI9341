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

	esp_audio_dec_info_t info{};
	mp3Loader(frameBuffer, FrameBufferLength, &info);
	mp3Loader.reset();

	ESP_LOGI(TAG, "sample rate = %dHz, bits = %dbit, %d channal", info.sample_rate, info.bits_per_sample, info.channel);

	GPIO{ GPIO::GPIO_NUM::GPIO_NUM_41 } = true;
	IIS iis{ GPIO_NUM_38, GPIO_NUM_39, GPIO_NUM_40, info.sample_rate, info.channel == 2 ? i2s_data_bit_width_t::I2S_DATA_BIT_WIDTH_32BIT : i2s_data_bit_width_t::I2S_DATA_BIT_WIDTH_16BIT };

	while (true)
	{
		auto size = mp3Loader(frameBuffer, FrameBufferLength);

		if (size == 0) break;

		iis.transmit(frameBuffer, size, portMAX_DELAY);
	}

	delete[] frameBuffer;

	MP3::deinit();
}
