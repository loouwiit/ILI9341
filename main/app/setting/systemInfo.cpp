#include "systemInfo.hpp"

#include "esp_chip_info.h"
#include "esp_flash.h"

void SystemInfo::init()
{
	App::init();

	taskListBuffer = new char[TaskListBufferSize] {'\0'};

	title.position.x -= title.computeSize().x / 2;
	title.computeSize();
	title.clickCallbackParam = this;
	title.releaseCallback = [](Finger&, void* param) { SystemInfo& self = *(SystemInfo*)param; self.exitCallback(nullptr); };

	for (unsigned char i = 0; i < SettingSize; i++)
	{
		settings[i].text = SettingName[i];
		settings[i].textColor = LCD::Color::White;
		settings[i].backgroundColor = BackgroundColor;
		settings[i].scale = TextSize;
		contents[i] = &settings[i];
	}
	settings[SettingSize - 1].text = taskListBuffer;
	settings[SettingSize - 1].scale = TaskTextSize;

	settings[0].position.x = ContentXOffset;
	settings[0].position.y = title.position.y + title.getSize().y + GapSize;
	for (unsigned char i = 1; i < SettingSize; i++)
	{
		settings[i].position.x = ContentXOffset;
		settings[i].position.y = settings[i - 1].position.y + settings[i - 1].computeSize().y + GapSize;
	}
	settings[SettingSize - 1].computeSize();

	contents[SettingSize] = &title;

	settings[SettingSize - 3].releaseCallback = [](Finger&, void*) { printInfo(); };

	snprintf(socBuffer + 12, sizeof(socBuffer) - 12, "%dMHz", CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ);

	xTaskCreate([](void* param)
		{
			SystemInfo& self = *(SystemInfo*)param;

			while (self.running)
			{
				snprintf(self.ramBuffer + 5, sizeof(self.psramBuffer) - 5, "%dKB", heap_caps_get_free_size(MALLOC_CAP_INTERNAL) / 1024);
				snprintf(self.psramBuffer + 7, sizeof(self.psramBuffer) - 7, "%dKB", heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024);
				vTaskList(self.taskListBuffer);
				vTaskDelay(1000);
			}

			delete[] self.taskListBuffer;
			self.taskListBuffer = nullptr;
			self.deleteAble = true;
			vTaskDelete(nullptr);
		}
	, "systemInfo", 4096, this, 2, nullptr);
}

void SystemInfo::deinit()
{
	running = false;
}

void SystemInfo::draw()
{
	lcd.clear();
	lcd.draw(contents);
}

void SystemInfo::touchUpdate()
{
	Finger finger[2] = { touch[0],touch[1] };

	if (finger[0].state == Finger::State::Press)
	{
		fingerActive[0] = true;
		lastFingerPosition[0] = finger[0].position;
		fingerMoveTotol[0] = {};
	}
	else if (finger[0].state == Finger::State::Realease)
	{
		if (abs2(fingerMoveTotol[0]) < moveThreshold2)
			click(finger[0]);
		fingerActive[0] = false;
		releaseDetect();
	}

	if (finger[1].state == Finger::State::Press)
	{
		fingerActive[1] = true;
		lastFingerPosition[1] = finger[1].position;
		fingerMoveTotol[1] = {};
	}
	else if (finger[1].state == Finger::State::Realease)
	{
		if (abs2(fingerMoveTotol[1]) < moveThreshold2)
			click(finger[1]);
		fingerActive[1] = false;
		releaseDetect();
	}

	if (fingerActive[0])
	{
		auto movement = finger[0].position - lastFingerPosition[0];
		fingerMoveTotol[0] += movement;
		offset += movement.y;
		lastFingerPosition[0] = finger[0].position;
	}
	if (fingerActive[1])
	{
		auto movement = finger[1].position - lastFingerPosition[1];
		fingerMoveTotol[1] += movement;
		offset += movement.y;
		lastFingerPosition[1] = finger[1].position;
	}
}

void SystemInfo::back()
{
	exitCallback(nullptr);
}

void SystemInfo::click(Finger finger)
{
	contents.finger(finger);
}

void SystemInfo::releaseDetect()
{
	if (touch[0].state != Finger::State::Contact && touch[1].state != Finger::State::Contact)
	{
		if (offset > 0)
			offset = 0;
	}
}

void SystemInfo::printInfo()
{
	/* Print chip information */
	esp_chip_info_t chip_info;
	uint32_t flash_size;
	esp_chip_info(&chip_info);
	printf("This is %s chip with %d CPU core(s), WiFi%s%s%s, ",
		CONFIG_IDF_TARGET,
		chip_info.cores,
		(chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
		(chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
		(chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

	unsigned major_rev = chip_info.revision / 100;
	unsigned minor_rev = chip_info.revision % 100;
	printf("silicon revision v%d.%d, ", major_rev, minor_rev);
	if (esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
		printf("Get flash size failed");
		return;
	}

	printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
		(chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

	char* taskListBuffer = new char[TaskListBufferSize];
	vTaskList(taskListBuffer);
	printf("%s", taskListBuffer);
	delete[] taskListBuffer;

	printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

	printf("default cap free: %d\n", heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
	heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
	printf("internal cap free: %d\n", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
	heap_caps_print_heap_info(MALLOC_CAP_INTERNAL);
	printf("spiram cap free: %d\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
	heap_caps_print_heap_info(MALLOC_CAP_SPIRAM);
}
