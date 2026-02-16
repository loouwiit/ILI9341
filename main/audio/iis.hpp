#pragma once

#include <algorithm>

#include <driver/i2s_std.h>

#include "gpio.hpp"

class IIS
{
public:
	IIS() = default;
	IIS(IIS&& move) { operator=(std::move(move)); }
	IIS& operator=(IIS&& move) { std::swap(move.tx_handle, tx_handle); return *this; }

	IIS(GPIO BCLK, GPIO DOUT, GPIO WS, uint32_t sampleRate)
	{
		i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
		chan_cfg.allow_pd = true;

		i2s_new_channel(&chan_cfg, &tx_handle, NULL);

		std_cfg.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(sampleRate);

		std_cfg.slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO);

		std_cfg.gpio_cfg = {
			.mclk = I2S_GPIO_UNUSED,
			.bclk = BCLK,
			.ws = WS,
			.dout = DOUT,
			.din = I2S_GPIO_UNUSED,
			.invert_flags = {
				.mclk_inv = false,
				.bclk_inv = false,
				.ws_inv = false,
			},
		};
		i2s_channel_init_std_mode(tx_handle, &std_cfg);
		i2s_channel_enable(tx_handle);
	}

	~IIS()
	{
		if (tx_handle == nullptr) return;
		i2s_channel_disable(tx_handle);
		i2s_del_channel(tx_handle);
		tx_handle = nullptr;
	}

	void setCallback(i2s_isr_callback_t callback, void* param)
	{
		i2s_event_callbacks_t cbs{};
		cbs.on_sent = callback;
		i2s_channel_register_event_callback(tx_handle, &cbs, param);
	}

	void changeSampleRate(uint32_t sampleRate)
	{
		i2s_channel_disable(tx_handle);
		std_cfg.clk_cfg.sample_rate_hz = sampleRate;
		i2s_channel_reconfig_std_clock(tx_handle, &std_cfg.clk_cfg);
	}

	size_t transmit(const void* buffer, size_t size, uint32_t timeOut = 1000)
	{
		size_t written = 0;
		auto ret = i2s_channel_write(tx_handle, buffer, size, &written, timeOut);
		ESP_ERROR_CHECK(ret);

		return written;
	}

private:
	i2s_chan_handle_t tx_handle{};
	i2s_std_config_t std_cfg{};
};
