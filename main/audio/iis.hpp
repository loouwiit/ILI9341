#pragma once

#include <algorithm>

#include <driver/i2s_std.h>

#include "gpio.hpp"

class IIS
{
public:
	IIS() = default;
	IIS(IIS&& move) { operator=(std::move(move)); }
	IIS& operator=(IIS&& move) { std::swap(move.tx_handle, tx_handle); std::swap(move.std_cfg, std_cfg); return *this; }

	IIS(GPIO BCLK, GPIO DOUT, GPIO WS, uint32_t sampleRate, i2s_data_bit_width_t bitWidth)
	{
		i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
		// chan_cfg.allow_pd = true; // E (10696) i2s_common: i2s_new_channel(968): register back up is not supported

		i2s_new_channel(&chan_cfg, &tx_handle, NULL);

		std_cfg.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(sampleRate);

		std_cfg.slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(bitWidth, I2S_SLOT_MODE_MONO);

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
	}

	~IIS()
	{
		if (tx_handle == nullptr) return;
		i2s_channel_disable(tx_handle);
		i2s_del_channel(tx_handle);
		tx_handle = nullptr;
	}

	bool isInited()
	{
		return tx_handle != nullptr;
	}

	bool isStarted()
	{
		return enabled;
	}

	void start()
	{
		if (isStarted()) return;
		enabled = true;
		i2s_channel_enable(tx_handle);
	}

	void stop()
	{
		if (!isStarted()) return;
		enabled = false;
		i2s_channel_disable(tx_handle);
	}

	void setCallback(i2s_isr_callback_t callback, void* param)
	{
		i2s_event_callbacks_t cbs{};
		cbs.on_sent = callback;
		i2s_channel_register_event_callback(tx_handle, &cbs, param);
	}

	void setSampleRate(uint32_t sampleRate)
	{
		std_cfg.clk_cfg.sample_rate_hz = sampleRate;
		i2s_channel_reconfig_std_clock(tx_handle, &std_cfg.clk_cfg);
	}

	void setBitWidth(i2s_data_bit_width_t bit_width)
	{
		std_cfg.slot_cfg.data_bit_width = bit_width;
		std_cfg.slot_cfg.ws_width = bit_width;
		i2s_channel_reconfig_std_slot(tx_handle, &std_cfg.slot_cfg);
	}

	void setSlotMode(i2s_slot_mode_t mono_or_stereo)
	{
		std_cfg.slot_cfg.slot_mode = mono_or_stereo;
		i2s_channel_reconfig_std_slot(tx_handle, &std_cfg.slot_cfg);
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
	bool enabled = false;
};
