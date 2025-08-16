#pragma once

#include <driver/spi_master.h>
#include <esp_task.h>
#include "gpio.hpp"

// only for transmit

// SPI
//  | -- SPIDevice
//  | -- SPIDevice
//  | -- SPIDevice

class SPI
{
public:
	using SpiHost = spi_host_device_t;

	SPI() = default;
	SPI(SpiHost host, GPIO MISO, GPIO MOSI, GPIO CLOCK);

	SPI(SPI&) = delete;
	SPI& operator=(SPI&) = delete;

	SPI(SPI&& move);
	SPI& operator=(SPI&& move);

	~SPI();

	operator SpiHost() { return host; }

private:
	SpiHost host = SPI_HOST_MAX;
};

class SPIDevice
{
public:
	using function_t = void(*)(void*);

	class SmallData_t {
	public:
		char operator[](auto i) { return data[i]; }
		char data[4];
	};

	static void IRAM_ATTR emptyCallback(void*) {};

	using waitFunction_t = bool (*)();
	static bool defaultWait() { return true; }
	static bool sleepWait() { vTaskDelay(1); return true; }

	SPIDevice() = default;
	SPIDevice(SPI& host, GPIO CS = GPIO::NC, unsigned char transmitionSize = 4, int speed = SPI_MASTER_FREQ_8M);

	SPIDevice(SPIDevice&) = delete;
	SPIDevice& operator=(SPIDevice&) = delete;

	SPIDevice(SPIDevice&& move);
	SPIDevice& operator=(SPIDevice&& move);
	~SPIDevice();

	bool transmit(const void* data, size_t sizeInBit, function_t callbackBefore = emptyCallback, void* callbackBeforeData = nullptr, function_t callbackAfter = emptyCallback, void* callbackAfterData = nullptr);
	bool transmit(SmallData_t data, size_t sizeInBit, function_t callbackBefore = emptyCallback, void* callbackBeforeData = nullptr, function_t callbackAfter = emptyCallback, void* callbackAfterData = nullptr);
	unsigned char getTransmittingCount();

	void waitForTransmition(waitFunction_t waitFunction = defaultWait);

protected:
	class Transmition
	{
	public:
		spi_transaction_t transmition{};
		bool transmitting = false;

		SPIDevice* spiDevice = nullptr;

		function_t callbackBefore = emptyCallback;
		function_t callbackAfter = emptyCallback;
		void* callbackBeforeData = nullptr;
		void* callbackAfterData = nullptr;

		operator spi_transaction_t& () { return transmition; }
		operator bool() { return transmitting; }
	};

	static void IRAM_ATTR spiDeviceStartCallback(spi_transaction_t* trans);
	static void IRAM_ATTR spiDeviceFinishCallback(spi_transaction_t* trans);

	spi_device_handle_t device = nullptr;

	Transmition* transmition = nullptr;
	unsigned char transmitionSize = 0;
	unsigned char transmitionIndex = 0;
	unsigned char transmitionCount = 0;
};
