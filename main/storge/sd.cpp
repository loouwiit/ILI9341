#include "sd.hpp"
#include "driver/sdmmc_host.h"

#include <esp_log.h>

#include "vfs.hpp"
#include "fat.hpp"
#include <sdmmc_cmd.h>

constexpr static char TAG[] = "SD";
EXT_RAM_BSS_ATTR static sdmmc_card_t* card = nullptr;

bool mountSd(SPI& spi, GPIO cs)
{
	if (!testFloor(PrefixSd))
	{
		ESP_LOGW(TAG, "%s is not exsit, can't mount sd card", PrefixSd);
		return false;
	}

	if (card != nullptr)
	{
		ESP_LOGW(TAG, "already mounted, don't mount again");
		return false;
	}

	cs.setPull(GPIO::Pull::GPIO_PULLUP_ONLY);

	esp_err_t ret;

	esp_vfs_fat_sdmmc_mount_config_t mount_config{};

	mount_config.format_if_mount_failed = false;
	mount_config.max_files = 5;
	mount_config.allocation_unit_size = 16 * 1024;

	ESP_LOGI(TAG, "Initializing SD card");

	ESP_LOGI(TAG, "Using SPI peripheral");

	sdmmc_host_t host = SDSPI_HOST_DEFAULT();

	sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
	slot_config.gpio_cs = cs;
	slot_config.host_id = spi;

	ESP_LOGI(TAG, "Mounting filesystem");
	ret = esp_vfs_fat_sdspi_mount(PrefixSd, &host, &slot_config, &mount_config, &card);

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount filesystem. "
				"If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
		}
		else {
			ESP_LOGE(TAG, "Failed to initialize the card (%s). "
				"Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
#ifdef CONFIG_EXAMPLE_DEBUG_PIN_CONNECTIONS
			check_sd_card_pins(&config, pin_count);
#endif
		}
		return false;
	}
	ESP_LOGI(TAG, "Filesystem mounted");

	sdmmc_card_print_info(stdout, card);
	return true;
}

void unmountSd()
{
	if (card == nullptr)
	{
		ESP_LOGW(TAG, "sd not mount, can't unmount");
		return;
	}
	esp_vfs_fat_sdcard_unmount(PrefixSd, card);
	card = nullptr;
	ESP_LOGI(TAG, "Card unmounted");
}
