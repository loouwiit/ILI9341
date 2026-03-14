#include <esp_ae_alc.h>

class ALC
{
public:
	ALC() = default;

	ALC(uint32_t sampleRate, uint8_t channel, uint8_t bitDepth = 16)
	{
		config.sample_rate = sampleRate;
		config.channel = channel;
		config.bits_per_sample = bitDepth;
		esp_ae_alc_open(&config, &handle);
	}

	~ALC()
	{
		if (handle != nullptr)
			esp_ae_alc_close(handle);
	}

	auto& operator=(ALC&& move)
	{
		std::swap(move.config, config);
		std::swap(move.handle, handle);
		std::swap(move.gainDb, gainDb);
		return *this;
	}

	void reset()
	{
		esp_ae_alc_reset(handle);
	}

	void setGain(int8_t gainDb)
	{
		ALC::gainDb = gainDb;
		for (uint8_t i = 0; i < config.channel; i++)
			esp_ae_alc_set_gain(handle, i, gainDb);
	}

	auto getGain()
	{
		return gainDb;
	}

	void process(void* in, void* out, size_t size)
	{
		auto sample_num = size / (config.channel * (config.bits_per_sample >> 3));
		esp_ae_alc_process(handle, sample_num, in, out);
	}

	void operator()(void* in, void* out, size_t size)
	{
		process(in, out, size);
	}

private:
	esp_ae_alc_cfg_t config{};
	esp_ae_alc_handle_t handle = nullptr;
	int8_t gainDb{};
};
