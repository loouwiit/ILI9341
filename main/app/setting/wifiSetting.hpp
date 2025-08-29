#pragma once

#include "app.hpp"
#include "esp_wifi_types_generic.h"

class WifiSetting final : public App
{
public:
	WifiSetting(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback) : App(lcd, touch, changeAppCallback, newAppCallback) {}

	virtual void init() override final;
	virtual void deinit() override final;

	virtual void draw() override final;
	virtual void touchUpdate() override final;
	virtual void back() override final;

private:
	constexpr static unsigned char TitleSize = 3;
	constexpr static unsigned char TextSize = 2;
	constexpr static short GapSize = 8;
	constexpr static short ContentXOffset = 20;
	constexpr static LCD::Color BackgroundColor = { 8,8,8 };

	constexpr static unsigned char ContensSize = 5;
	LCD::Layar<LayarClassicSize::Small> contents{ ContensSize };
	LCD::Text title{ {LCD::ScreenSize.x / 2, 0}, "wifi setting", TitleSize };

	constexpr static unsigned char SwitchSize = 3;
	constexpr static const char* SwitchName[SwitchSize] = { "(de)init wifi" ,"ap:state", "wifi:state" };
	LCD::Layar<LayarClassicSize::Small> switchLayar{ SwitchSize };
	LCD::Text switchs[SwitchSize]{};

	constexpr static unsigned char ApSettingSize = 3;
	constexpr static const char* ApSettingName[ApSettingSize] = { "access point", "ssid:error", "password:error" };
	LCD::Layar<LayarClassicSize::Small> apSettingLayar{ ApSettingSize };
	LCD::Text apSettings[ApSettingSize]{};

	constexpr static unsigned char WifiSettingSize = 4;
	constexpr static const char* WifiSettingName[WifiSettingSize] = { "wifi connect", "ssid:error", "password:error", "ip: error" };
	char ipBuffer[19] = "ip:255.255.255.255";
	LCD::Layar<LayarClassicSize::Small> wifiSettingLayar{ WifiSettingSize };
	LCD::Text wifiSettings[WifiSettingSize]{};

	constexpr static unsigned char WifiScanSize = 2;
	constexpr static unsigned char WifiListSize = 20;
	LCD::Layar<LayarClassicSize::Pair> wifiScanLayar{ WifiScanSize };
	LCD::Text wifiScanText{ {0,0}, "wifi scan", TextSize, LCD::Color::White, BackgroundColor };
	LCD::Layar<LayarClassicSize::Large> wifiListLayar{ WifiListSize };
	LCD::Text wifiListText[WifiListSize]{};
	wifi_ap_record_t wifiListBuffer[WifiListSize]{};
	struct WifiListCallbackParam_t { WifiSetting* self; unsigned char index; };
	WifiListCallbackParam_t wifiListCallbackParam[WifiListSize];

	constexpr static float moveThreshold2 = 100.0f;

	short& offset = contents.start.y;
	bool fingerActive[2] = { false, false };
	Vector2s lastFingerPosition[2]{};
	Vector2s fingerMoveTotol[2]{};

	void click(Finger finger);
	void releaseDetect();

	void updateLayar();
	void updateSwitch();
	void updateIp();
	void wifiListClickd(unsigned char index);

	using CoThreadFunction_t = void(*)(WifiSetting& self);
	constexpr static size_t CoThreadQueueLength = 4;

	QueueHandle_t coThreadQueue = nullptr;
	static void coThread(void* param);
	bool coThreadDeal(CoThreadFunction_t function);

	static void scanWifi(WifiSetting& self);

	static bool ssidInputChecker(char* ssid);
	static bool passwordInputChecker(char* password);

	static void textComputeSizer(void* param);
};
