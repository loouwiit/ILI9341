#include <mdns.h>
#include "mdns.hpp"

#include <esp_log.h>

constexpr static char TAG[] = "mdns";

void mdnsStart(const char* name)
{
	esp_err_t err = mdns_init();
	if (err) {
		ESP_LOGE(TAG, "init failed: %d\n", err);
		return;
	}

	mdns_hostname_set(name);
	mdns_instance_name_set("ESP32S3 server");
	mdns_service_add(nullptr, "_http", "_tcp", 80, nullptr, 0);

	ESP_LOGI(TAG, "started");
}

void mdnsStop()
{
	mdns_free();
	ESP_LOGI(TAG, "stopped");
}
