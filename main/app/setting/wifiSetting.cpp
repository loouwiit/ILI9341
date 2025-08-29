#include "wifiSetting.hpp"

#include <cstring>

#include "wifi/wifi.hpp"
#include "wifi/mdns.hpp"
#include "input/input.hpp"

constexpr char TAG[] = "WifiSetting";

static char apSettingSsidTextBuffer[6 + 32 + 1] = "ssid:";
static char* apSettingSsid = apSettingSsidTextBuffer + 5;
static char apSettingPasswordTextBuffer[10 + 64 + 1] = "password:";
static char* apSettingPassword = apSettingPasswordTextBuffer + 9;

static char wifiSettingSsidTextBuffer[6 + 32 + 1] = "ssid:";
static char* wifiSettingSsid = wifiSettingSsidTextBuffer + 5;
static char wifiSettingPasswordTextBuffer[10 + 64 + 1] = "password:";
static char* wifiSettingPassword = wifiSettingPasswordTextBuffer + 9;

void WifiSetting::init()
{
	App::init();

	coThreadQueue = xQueueCreate(CoThreadQueueLength, sizeof(CoThreadFunction_t));
	if (xTaskCreate(coThread, "wifiSettingCothread", 4096, this, 2, nullptr) != pdTRUE)
	{
		wifiScanText.text = "error:out of memory";
		deleteAble = true;
	}

	title.position.x -= title.computeSize().x / 2;
	title.computeSize();
	title.clickCallbackParam = this;
	title.releaseCallback = [](Finger&, void* param) { WifiSetting& self = *(WifiSetting*)param; self.changeAppCallback(nullptr); };

	contents[0] = &title;
	contents[1] = &switchLayar;
	contents[2] = &apSettingLayar;
	contents[3] = &wifiSettingLayar;
	contents[4] = &wifiScanLayar;

	{
		for (unsigned char i = 0; i < SwitchSize; i++)
		{
			switchs[i].text = SwitchName[i];
			switchs[i].textColor = LCD::Color::White;
			switchs[i].backgroundColor = BackgroundColor;
			switchs[i].scale = TextSize;
			switchLayar[i] = &switchs[i];
		}

		for (unsigned char i = 1; i < SwitchSize; i++)
			switchs[i].position.y = switchs[i - 1].position.y + switchs[i - 1].computeSize().y + GapSize;
		switchs[SwitchSize - 1].computeSize();

		switchLayar.start.x = ContentXOffset;
		switchLayar.end.x = LCD::ScreenSize.x;

		switchs[0].clickCallbackParam = this;
		switchs[0].releaseCallback = [](Finger&, void* param)
			{
				WifiSetting& self = *(WifiSetting*)param;
				if (wifiIsInited())
				{
					if (!wifiIsStarted())
					{
						mdnsStop();
						wifiDeinit();
					}
				}
				else
				{
					wifiInit();
					mdnsStart();
					wifiNatSetAutoStart();
				}
				self.updateSwitch();
				self.updateIp();
				self.updateLayar();
			};

		switchs[1].clickCallbackParam = this;
		switchs[1].releaseCallback = [](Finger&, void* param)
			{
				WifiSetting& self = *(WifiSetting*)param;
				if (wifiApIsStarted())
				{
					wifiApStop();
					if (!wifiStationIsStarted())
						wifiStop();
				}
				else
				{
					wifiApStart();
					if (!wifiIsStarted())
						wifiStart();
				}
				self.updateSwitch();
				self.updateIp();
				self.updateLayar();
			};
		switchs[2].clickCallbackParam = this;
		switchs[2].releaseCallback = [](Finger&, void* param)
			{
				WifiSetting& self = *(WifiSetting*)param;
				if (wifiStationIsStarted())
				{
					wifiStationStop();
					if (!wifiApIsStarted())
						wifiStop();
				}
				else
				{
					wifiStationStart();
					if (!wifiIsStarted())
						wifiStart();
				}
				self.updateSwitch();
				self.updateIp();
				self.updateLayar();
			};
	}

	{
		for (unsigned char i = 0; i < ApSettingSize; i++)
		{
			apSettings[i].text = ApSettingName[i];
			apSettings[i].textColor = LCD::Color::White;
			apSettings[i].backgroundColor = BackgroundColor;
			apSettings[i].scale = TextSize;
			apSettingLayar[i] = &apSettings[i];
		}
		apSettings[1].text = apSettingSsidTextBuffer;
		apSettings[2].text = apSettingPasswordTextBuffer;

		for (unsigned char i = 1; i < ApSettingSize; i++)
			apSettings[i].position.y = apSettings[i - 1].position.y + apSettings[i - 1].computeSize().y + GapSize;
		apSettings[ApSettingSize - 1].computeSize();

		apSettingLayar.start.x = ContentXOffset;
		apSettingLayar.end.x = LCD::ScreenSize.x;

		apSettings[0].clickCallbackParam = this;
		apSettings[0].releaseCallback = [](Finger&, void* param)
			{
				WifiSetting& self = *(WifiSetting*)param;
				self.coThreadDeal([](WifiSetting& self)
					{
						wifiApSet(apSettingSsid, apSettingPassword);
						if (wifiApIsStarted())
							wifiApStop();
						wifiApStart();
						self.updateSwitch();
						self.updateIp();
						while (wifiIsWantConnect() && !wifiIsConnect())
							vTaskDelay(100);
						self.updateSwitch();
						self.updateIp();
					});
			};
		apSettings[1].clickCallbackParam = this;
		apSettings[1].releaseCallback = [](Finger&, void* param)
			{
				WifiSetting& self = *(WifiSetting*)param;
				AppInput* input = new AppInput{ self.lcd, self.touch, self.changeAppCallback, self.newAppCallback };
				input->setInputBuffer(apSettingSsid);
				input->checker = ssidInputChecker;
				input->finishCallbackParam = &self.apSettings[1];
				input->finishCallback = textComputeSizer;
				self.newAppCallback(input);
			};
		apSettings[2].clickCallbackParam = this;
		apSettings[2].releaseCallback = [](Finger&, void* param)
			{
				WifiSetting& self = *(WifiSetting*)param;
				AppInput* input = new AppInput{ self.lcd, self.touch, self.changeAppCallback, self.newAppCallback };
				input->setInputBuffer(apSettingPassword);
				input->checker = passwordInputChecker;
				input->finishCallbackParam = &self.apSettings[2];
				input->finishCallback = textComputeSizer;
				self.newAppCallback(input);
			};
	}

	{
		for (unsigned char i = 0; i < WifiSettingSize; i++)
		{
			wifiSettings[i].text = WifiSettingName[i];
			wifiSettings[i].textColor = LCD::Color::White;
			wifiSettings[i].backgroundColor = BackgroundColor;
			wifiSettings[i].scale = TextSize;
			wifiSettingLayar[i] = &wifiSettings[i];
		}
		wifiSettings[1].text = wifiSettingSsidTextBuffer;
		wifiSettings[2].text = wifiSettingPasswordTextBuffer;

		for (unsigned char i = 1; i < WifiSettingSize; i++)
			wifiSettings[i].position.y = wifiSettings[i - 1].position.y + wifiSettings[i - 1].computeSize().y + GapSize;
		wifiSettings[WifiSettingSize - 1].computeSize();

		wifiSettingLayar.start.x = ContentXOffset;
		wifiSettingLayar.end.x = LCD::ScreenSize.x;

		wifiSettings[0].clickCallbackParam = this;
		wifiSettings[0].releaseCallback = [](Finger&, void* param)
			{
				WifiSetting& self = *(WifiSetting*)param;
				self.coThreadDeal([](WifiSetting& self)
					{
						self.switchs[2].text = "wifi:setting";
						wifiConnect(wifiSettingSsid, wifiSettingPassword);
						self.updateSwitch();
						self.updateIp();
						while (wifiIsWantConnect() && !wifiIsConnect())
							vTaskDelay(100);
						self.updateSwitch();
						self.updateIp();
					});
			};
		wifiSettings[1].clickCallbackParam = this;
		wifiSettings[1].releaseCallback = [](Finger&, void* param)
			{
				WifiSetting& self = *(WifiSetting*)param;
				AppInput* input = new AppInput{ self.lcd, self.touch, self.changeAppCallback, self.newAppCallback };
				input->setInputBuffer(wifiSettingSsid);
				input->checker = ssidInputChecker;
				input->finishCallbackParam = &self.wifiSettings[1];
				input->finishCallback = textComputeSizer;
				self.newAppCallback(input);
			};
		wifiSettings[2].clickCallbackParam = this;
		wifiSettings[2].releaseCallback = [](Finger&, void* param)
			{
				WifiSetting& self = *(WifiSetting*)param;
				AppInput* input = new AppInput{ self.lcd, self.touch, self.changeAppCallback, self.newAppCallback };
				input->setInputBuffer(wifiSettingPassword);
				input->checker = passwordInputChecker;
				input->finishCallbackParam = &self.wifiSettings[2];
				input->finishCallback = textComputeSizer;
				self.newAppCallback(input);
			};

		wifiSettings[3].text = ipBuffer;
	}

	{
		wifiScanLayar[0] = &wifiScanText;
		wifiScanLayar[1] = &wifiListLayar;

		for (unsigned char i = 0; i < WifiListSize; i++)
		{
			wifiListText[i].text = (const char*)wifiListBuffer[i].ssid;
			wifiListText[i].textColor = LCD::Color::White;
			wifiListText[i].backgroundColor = BackgroundColor;
			wifiListText[i].scale = TextSize;
			wifiListLayar[i] = &wifiListText[i];
		}

		for (unsigned char i = 1; i < WifiListSize; i++)
			wifiListText[i].position.y = wifiListText[i - 1].position.y + wifiListText[i - 1].computeSize().y + GapSize;
		wifiListText[WifiListSize - 1].computeSize();

		wifiScanLayar.start.x = ContentXOffset;
		wifiScanLayar.end.x = LCD::ScreenSize.x;

		wifiListLayar.start.x = 0;
		wifiListLayar.end.x = LCD::ScreenSize.x;
		wifiListLayar.start.y = wifiScanText.position.y + wifiScanText.computeSize().y + GapSize;
		wifiListLayar.end.y = (short)32767;

		wifiScanText.clickCallbackParam = this;
		wifiScanText.releaseCallback = [](Finger&, void* param)
			{
				WifiSetting& self = *(WifiSetting*)param;
				ESP_LOGI(TAG, "wifi scan");
				self.coThreadDeal(scanWifi);
			};

		for (unsigned char i = 0; i < WifiListSize; i++)
		{
			wifiListCallbackParam[i].self = this;
			wifiListCallbackParam[i].index = i;
			wifiListText[i].clickCallbackParam = &wifiListCallbackParam[i];
			wifiListText[i].releaseCallback = [](Finger&, void* param)
				{
					WifiSetting& self = *((WifiListCallbackParam_t*)param)->self;
					unsigned char& index = ((WifiListCallbackParam_t*)param)->index;
					self.wifiListClickd(index);
				};
		}
	}

	updateSwitch();
	updateIp();
	updateLayar();
}

void WifiSetting::deinit()
{
	running = false;

	while (coThreadDeal(nullptr))
		vTaskDelay(1);

	while (!deleteAble)
		vTaskDelay(1);

	vQueueDelete(coThreadQueue);
}

void WifiSetting::draw()
{
	lcd.clear();
	lcd.draw(contents);
}

void WifiSetting::touchUpdate()
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

void WifiSetting::back()
{
	changeAppCallback(nullptr);
}

void WifiSetting::click(Finger finger)
{
	contents.finger(finger);
}

void WifiSetting::releaseDetect()
{
	if (touch[0].state != Finger::State::Contact && touch[1].state != Finger::State::Contact)
	{
		if (offset > 0)
			offset = 0;
	}
}

void WifiSetting::updateLayar()
{
	short nowY = title.position.y + title.getSize().y;
	nowY += GapSize;

	if (switchLayar.elementCount != 0)
	{
		switchLayar.start.y = nowY;
		nowY = switchLayar.end.y = switchLayar.start.y + switchs[SwitchSize - 1].position.y + switchs[SwitchSize - 1].getSize().y;
		nowY += GapSize + 16 * TextSize;
	}

	if (apSettingLayar.elementCount != 0)
	{
		apSettingLayar.start.y = nowY;
		nowY = apSettingLayar.end.y = apSettingLayar.start.y + apSettings[ApSettingSize - 1].position.y + apSettings[ApSettingSize - 1].getSize().y;
		nowY += GapSize + 16 * TextSize;
	}

	if (wifiSettingLayar.elementCount != 0)
	{
		wifiSettingLayar.start.y = nowY;
		nowY = wifiSettingLayar.end.y = wifiSettingLayar.start.y + wifiSettings[WifiSettingSize - 1].position.y + wifiSettings[WifiSettingSize - 1].getSize().y;
		nowY += GapSize + 16 * TextSize;
	}

	if (wifiScanLayar.elementCount != 0)
	{
		wifiScanLayar.start.y = nowY;
		wifiScanLayar.end.y = (short)32767;
	}
}

void WifiSetting::updateSwitch()
{
	switchs[0].text = wifiIsInited() ? "deinit wifi" : "init wifi";
	switchs[0].computeSize();
	switchs[1].text = wifiApIsStarted() ? "ap:on" : "ap:off";
	switchs[1].computeSize();
	switchs[2].text = wifiStationIsStarted() ? (wifiIsConnect() ? "wifi:connected" : (wifiIsWantConnect() ? "wifi:connecting" : "wifi:disconnected")) : "wifi:off"; // 好屎💩
	switchs[2].computeSize();

	switchLayar.elementCount = wifiIsInited() ? SwitchSize : 1;

	apSettingLayar.elementCount = wifiApIsStarted() ? ApSettingSize : 0;
	wifiSettingLayar.elementCount = wifiStationIsStarted() ? WifiSettingSize : 0;
	wifiScanLayar.elementCount = wifiStationIsStarted() ? WifiScanSize : 0;
}

void WifiSetting::updateIp()
{
	auto ip = wifiStationGetIp();

	sprintf(ipBuffer, "ip:%lu.%lu.%lu.%lu", ip.addr & 0xFF, (ip.addr >> 8) & 0xFF, (ip.addr >> 16) & 0xFF, (ip.addr >> 24) & 0xFF);
}

void WifiSetting::wifiListClickd(unsigned char index)
{
	ESP_LOGI(TAG, "list click at %d", index);
	if (0 == strcmp(wifiSettingSsid, (const char*)wifiListBuffer[index].ssid))
		offset = -(wifiSettingLayar.start.y + wifiSettings[0].position.y);
	strcpy(wifiSettingSsid, (const char*)wifiListBuffer[index].ssid);
}

void WifiSetting::coThread(void* param)
{
	WifiSetting& self = *(WifiSetting*)param;
	QueueHandle_t& queue = self.coThreadQueue;
	CoThreadFunction_t function;

	while (self.running)
	{
		while (xQueueReceive(queue, &function, portMAX_DELAY) != pdTRUE) {}
		if (function == nullptr) continue;
		function(self);
	}

	self.deleteAble = true;
	vTaskDelete(nullptr);
}

bool WifiSetting::coThreadDeal(CoThreadFunction_t function)
{
	return pdTRUE == xQueueSend(coThreadQueue, &function, 0);
}

void WifiSetting::scanWifi(WifiSetting& self)
{
	unsigned char count = wifiStationScan(self.wifiListBuffer, WifiListSize);
	count = std::min(count, WifiListSize);

	for (unsigned char i = 0; i < count; i++)
		self.wifiListText[i].computeSize();

	self.wifiListLayar.elementCount = count;
}

bool WifiSetting::ssidInputChecker(char* ssid)
{
	return ssid[32] == '\0';
}

bool WifiSetting::passwordInputChecker(char* password)
{
	return password[64] == '\0';
}

void WifiSetting::textComputeSizer(void* param)
{
	LCD::Text& text = *(LCD::Text*)param;
	text.computeSize();
}
