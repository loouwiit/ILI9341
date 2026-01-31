#pragma once

#include "esp_log.h"

#include "led_strip.h"
#include "color.hpp"
#include "gpio.hpp"

class Strip
{
public:
	constexpr static uint32_t LED_STRIP_RMT_RES_HZ = 10 * 1000 * 1000;

	using RGB = Color888;

	struct RGBW
	{
		uint32_t r = 0;
		uint32_t g = 0;
		uint32_t b = 0;
		uint32_t w = 0;
	};

	struct HSV
	{
		uint16_t h = 0;
		uint8_t s = 0;
		uint8_t v = 0xFF;
	};

	class Led
	{
	public:
		Led() = default;

		RGB operator=(RGB rgb)
		{
			color.rgb = rgb;
			led_strip_set_pixel(handle, index, rgb.R, rgb.G, rgb.B);
			return rgb;
		}

		RGBW operator=(RGBW rgbw)
		{
			color.rgbw = rgbw;
			led_strip_set_pixel_rgbw(handle, index, rgbw.r, rgbw.g, rgbw.b, rgbw.w);
			return rgbw;
		}

		HSV operator=(HSV hsv)
		{
			color.hsv = hsv;
			led_strip_set_pixel_hsv(handle, index, hsv.h, hsv.s, hsv.v);
			return hsv;
		}

		operator RGB()
		{
			return color.rgb;
		}

		operator RGBW()
		{
			return color.rgbw;
		}

		operator HSV()
		{
			return color.hsv;
		}

	private:
		friend class Strip;

		Led(led_strip_handle_t handle, uint32_t index) :handle{ handle }, index{ index } {}

		union {
			RGB rgb;
			RGBW rgbw;
			HSV hsv;
		} color{};

		led_strip_handle_t handle = nullptr;
		uint32_t index = 0;
	};

	class Snapshot;
	class Manager;

	Strip() = default;
	Strip(Strip&& move)
	{
		operator=(std::move(move));
	}
	Strip& operator=(Strip&& move)
	{
		std::swap(move.handle, handle);
		std::swap(move.leds, leds);
		std::swap(move.ledCount, ledCount);
		return *this;
	}
	Strip(GPIO gpio, uint32_t ledCount, led_model_t model, led_color_component_format_t format = LED_STRIP_COLOR_COMPONENT_FMT_GRB) :
		ledCount{ ledCount }
	{
		// LED strip general initialization, according to your led board design
		led_strip_config_t strip_config{};
		strip_config.strip_gpio_num = gpio;   // The GPIO that connected to the LED strip's data line
		strip_config.max_leds = ledCount;        // The number of LEDs in the strip,
		strip_config.led_model = model;            // LED strip model
		strip_config.color_component_format = format; // Pixel format of your LED strip
		strip_config.flags.invert_out = false;                // whether to invert the output signal

		// LED strip backend configuration: RMT
		led_strip_rmt_config_t rmt_config{};
		rmt_config.clk_src = RMT_CLK_SRC_DEFAULT;        // different clock source can lead to different power consumption
		rmt_config.resolution_hz = LED_STRIP_RMT_RES_HZ; // RMT counter clock frequency
		rmt_config.flags.with_dma = true;               // DMA feature is available on ESP target like ESP32-S3

		// LED Strip object handle
		if (auto ret = led_strip_new_rmt_device(&strip_config, &rmt_config, &handle))
			ESP_ERROR_CHECK(ret);
		else
		{
			leds = new Led[ledCount];
			for (uint32_t i = 0; i < ledCount; i++)
			{
				leds[i].handle = handle;
				leds[i].index = i;
			}
		}
	}

	~Strip()
	{
		delete[] leds;
		leds = nullptr;

		if (handle == nullptr) return;
		led_strip_del(handle);
		handle = nullptr;
	}

	auto getCount()
	{
		return ledCount;
	}

	bool empty()
	{
		return handle == nullptr;
	}

	void flush()
	{
		led_strip_refresh(handle);
	}

	void clear()
	{
		led_strip_clear(handle);
	}

	Led& operator [](uint32_t i)
	{
		return leds[i];
	}

private:
	led_strip_handle_t handle = nullptr;
	Led* leds = nullptr;
	uint32_t ledCount = 0;
};
