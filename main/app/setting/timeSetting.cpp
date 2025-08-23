#include "timeSetting.hpp"

#include <esp_task.h>
#include <esp_netif_sntp.h>

constexpr static char TAG[] = "TimeSetting";

void TimeSetting::init()
{
	App::init();

	title.position.x -= title.computeSize().x / 2;
	title.computeSize();
	title.clickCallbackParam = this;
	title.releaseCallback = [](Finger&, void* param) { TimeSetting& self = *(TimeSetting*)param; self.changeAppCallback(nullptr); };

	contents[0] = &title;
	contents[1] = &nowDate;
	contents[2] = &nowTime;
	contents[3] = &ntpUpdate;

	nowDate.text = dateBuffer;
	nowTime.text = timeBuffer;

	ntpUpdate.computeSize();
	ntpUpdate.clickCallbackParam = this;
	ntpUpdate.releaseCallback = [](Finger&, void* param)
		{
			xTaskCreate([](void* param)
				{
					TimeSetting& self = *(TimeSetting*)param;

					self.syncMutex.lock();
					if (self.syncing)
					{
						self.syncMutex.unlock();
						vTaskDelete(nullptr);
					}
					self.syncing = true;
					self.syncMutex.unlock();

					ESP_LOGI(TAG, "syncing");
					esp_sntp_config_t sntpConfig = {
						.smooth_sync = false,
						.server_from_dhcp = true,
						.wait_for_sync = true,
						.start = true,
						.sync_cb = NULL,
						.renew_servers_after_new_IP = false,
						.ip_event_to_renew = IP_EVENT_STA_GOT_IP,
						.index_of_first_server = 1,
						.num_of_servers = 1,
						.servers = {"pool.ntp.org"},
					};
					esp_netif_sntp_init(&sntpConfig);
					while (ESP_ERR_TIMEOUT == esp_netif_sntp_sync_wait(100) && self.running);
					esp_netif_sntp_deinit();

					self.syncMutex.lock();
					self.syncing = false;
					self.syncMutex.unlock();

					vTaskDelete(nullptr);
				}, "ntp", 4096, param, 2, nullptr);
		};

	xTaskCreate(
		[](void* param)
		{
			TimeSetting& timeSetting = *(TimeSetting*)param;
			time_t lastTime = 0;
			time_t nowTime = 0;
			while (timeSetting.running)
			{
				vTaskDelay(10);
				nowTime = time(nullptr);
				if (nowTime == lastTime)
					continue;

				timeSetting.updateTime(nowTime);
				lastTime = nowTime;
			}
			timeSetting.deleteAble = true;
			vTaskDelete(nullptr);
		}
	, "timeSetting", 4096, this, 1, nullptr);
}

void TimeSetting::deinit()
{
	running = false;
	while (syncing)
		vTaskDelay(1);
}

void TimeSetting::draw()
{
	lcd.clear();
	lcd.draw(contents);
}

void TimeSetting::touchUpdate()
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

void TimeSetting::back()
{
	changeAppCallback(nullptr);
}

void TimeSetting::updateTime(time_t nowTime)
{
	tm* tm = localtime(&nowTime);

	strftime(dateBuffer, sizeof(dateBuffer), "%Y/%m/%d", tm);
	strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", tm);
}

void TimeSetting::click(Finger finger)
{
	contents.finger(finger);
}

void TimeSetting::releaseDetect()
{
	if (touch[0].state != Finger::State::Contact && touch[1].state != Finger::State::Contact)
	{
		if (offset > 0)
			offset = 0;
	}
}
