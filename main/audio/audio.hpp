#pragma once

// this file seems an hpp but actually act as cpp more likely
// so it's an debug file :)
#warning debug file compiled

#include <esp_mp3_dec.h>
#include <esp_audio_dec.h>

#include "storge/fat.hpp"

#include "driver/i2s_std.h"

IFile audioFile{};

constexpr size_t RawBufferLength = 1536;
esp_audio_dec_in_raw_t rawIn{};

using Mp3Handle_t = void*;
Mp3Handle_t handle = nullptr;

i2s_chan_handle_t tx_handle;

bool audioInit(const char* path)
{
	constexpr auto TAG = "audio";

	Floor root{};
	root.open(PerfixRoot);
	if (!root.openFile(path, strlen(path), audioFile)) { ESP_LOGE(TAG, "open %s failed", path); return false; }

	rawIn.buffer = new uint8_t[RawBufferLength];
	rawIn.frame_recover = esp_audio_dec_recovery_t::ESP_AUDIO_DEC_RECOVERY_NONE;

	esp_mp3_dec_register();

	esp_mp3_dec_open(nullptr, 0, &handle);

	return true;
}

void audioDeinit()
{
	esp_mp3_dec_close(handle);
	esp_audio_dec_unregister(ESP_AUDIO_TYPE_MP3);

	delete[] rawIn.buffer;
	rawIn.buffer = nullptr;

	audioFile.close();
}

size_t audioDecode(uint8_t* buffer, size_t bufferSize)
{
	constexpr auto TAG = "decode";

	rawIn.len = audioFile.read(rawIn.buffer, RawBufferLength);
	audioFile.setOffset(-rawIn.len, FileBase::OffsetMode::Current);

	esp_audio_dec_out_frame_t frameOut{};
	frameOut.buffer = buffer;
	frameOut.len = bufferSize;

	esp_audio_dec_info_t info{};

	auto ret = esp_mp3_dec_decode(handle, &rawIn, &frameOut, &info);
	if (ret != ESP_AUDIO_ERR_OK)
	{
		if (ret == ESP_AUDIO_ERR_BUFF_NOT_ENOUGH)
			ESP_LOGE(TAG, "decode need %d while has %d", frameOut.needed_size, frameOut.len);
		else ESP_LOGE(TAG, "error %d", ret);
	}

	ESP_LOGI(TAG, "decode %d from %d", frameOut.decoded_size, rawIn.consumed);
	audioFile.setOffset(rawIn.consumed, FileBase::OffsetMode::Current);

	return frameOut.decoded_size;
}

void iisInit()
{
	/* 通过辅助宏获取默认的通道配置
	 * 这个辅助宏在 'i2s_common.h' 中定义，由所有 I2S 通信模式共享
	 * 它可以帮助指定 I2S 角色和端口 ID */
	i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
	/* 分配新的 TX 通道并获取该通道的句柄 */
	i2s_new_channel(&chan_cfg, &tx_handle, NULL);

	/* 进行配置，可以通过宏生成声道配置和时钟配置
	 * 这两个辅助宏在 'i2s_std.h' 中定义，只能用于 STD 模式
	 * 它们可以帮助初始化或更新声道和时钟配置 */
	i2s_std_config_t std_cfg = {
		.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(44100),
		.slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
		.gpio_cfg = {
			.mclk = I2S_GPIO_UNUSED,
			.bclk = GPIO_NUM_38,
			.ws = GPIO_NUM_40,
			.dout = GPIO_NUM_39,
			.din = I2S_GPIO_UNUSED,
			.invert_flags = {
				.mclk_inv = false,
				.bclk_inv = false,
				.ws_inv = false,
			},
		},
	};
	/* 初始化通道 */
	i2s_channel_init_std_mode(tx_handle, &std_cfg);

	/* 在写入数据之前，先启用 TX 通道 */
	i2s_channel_enable(tx_handle);

	// 调节 gain
	GPIO{ GPIO::GPIO_NUM::GPIO_NUM_41 } = true;
}

void iisDeinit()
{
	/* 删除通道之前必须先禁用通道 */
	i2s_channel_disable(tx_handle);
	/* 如果不再需要句柄，删除该句柄以释放通道资源 */
	i2s_del_channel(tx_handle);
}

size_t iisTransmit(const uint8_t* buffer, size_t size)
{
	size_t written = 0;
	auto ret = i2s_channel_write(tx_handle, buffer, size, &written, portMAX_DELAY);
	ESP_ERROR_CHECK(ret);

	/* 如果需要更新声道或时钟配置
	 * 需要在更新前先禁用通道 */
	 // i2s_channel_disable(tx_handle);
	 // std_cfg.slot_cfg.slot_mode = I2S_SLOT_MODE_MONO; // 默认为立体声
	 // i2s_channel_reconfig_std_slot(tx_handle, &std_cfg.slot_cfg);
	 // std_cfg.clk_cfg.sample_rate_hz = 96000;
	 // i2s_channel_reconfig_std_clock(tx_handle, &std_cfg.clk_cfg);

	return written;
}

void audioTest(const char* path)
{
	constexpr auto TAG = "audio test";

	constexpr size_t FrameBufferLength = 8192;
	auto* frameBuffer = new uint8_t[FrameBufferLength];

	if (!audioInit(path)) return;
	iisInit();

	while (!audioFile.eof())
	{
		auto size = audioDecode(frameBuffer, FrameBufferLength);

		if (size == 0) break;

		auto transmitSize = iisTransmit(frameBuffer, size);
		ESP_LOGI(TAG, "transmited %d", transmitSize);
	}

	iisDeinit();
	audioDeinit();

	delete[] frameBuffer;
}
