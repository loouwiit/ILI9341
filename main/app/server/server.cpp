#include "server.hpp"

#include "serverKernal.hpp"
#include "tempture.hpp"

#include "wifi/wifi.hpp"
#include "app/setting/wifiSetting.hpp"

void AppServer::init()
{
	App::init();

	title.position = { LCD::ScreenSize.x / 2, 0 };
	title.position.x -= title.computeSize().x / 2;
	title.computeSize();
	title.clickCallbackParam = this;
	title.releaseCallback = [](Finger&, void* param) { AppServer& self = *(AppServer*)param; self.changeAppCallback(nullptr); };

	contents.elementCount = 4;
	contents[0] = &title;
	contents[1] = &server;
	contents[2] = &temptureInit;
	contents[3] = &temptureStart;

	if (!wifiIsInited())
	{
		contents.elementCount = 2;
		server.text = "wifi not inited";
		server.computeSize();
		server.clickCallbackParam = this;
		server.releaseCallback = [](Finger&, void* param)
			{
				AppServer& self = *(AppServer*)param;
				WifiSetting* wifiSetting = new WifiSetting{ self.lcd, self.touch, self.changeAppCallback, self.newAppCallback };
				self.newAppCallback(wifiSetting);
			};
		return;
	}

	updateState();

	server.clickCallbackParam = this;
	server.releaseCallback = [](Finger&, void* param)
		{
			AppServer& self = *(AppServer*)param;
			if (serverIsStarted())
				serverStop();
			else serverStart();
			self.updateState();
		};

	temptureInit.clickCallbackParam = this;
	temptureInit.releaseCallback = [](Finger&, void* param)
		{
			AppServer& self = *(AppServer*)param;
			if (temperatureIsInited())
			{
				if (!temperatureIsStarted())
					temperatureDeinit();
			}
			else temperatureInit();
			self.updateState();
		};

	temptureStart.clickCallbackParam = this;
	temptureStart.releaseCallback = [](Finger&, void* param)
		{
			AppServer& self = *(AppServer*)param;
			if (temperatureIsStarted())
				temperatureStop();
			else temperatureStart();
			self.updateState();
		};
}

void AppServer::focusIn()
{
	init();
}

void AppServer::draw()
{
	lcd.clear();
	lcd.draw(contents);
}

void AppServer::touchUpdate()
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

void AppServer::back()
{
	changeAppCallback(nullptr);
}

void AppServer::updateState()
{
	server.text = serverIsStarted() ? "server:started" : "server:stoped";
	server.computeSize();

	temptureInit.text = temperatureIsInited() ? "deinit tempture" : "init tempture";
	temptureInit.computeSize();

	if (!temperatureIsInited())
		temptureStart.text = "";
	else
		temptureStart.text = temperatureIsStarted() ? "tempture:started" : "tempture:stoped";
	temptureStart.computeSize();
}

void AppServer::click(Finger finger)
{
	contents.finger(finger);
}

void AppServer::releaseDetect()
{
	if (touch[0].state != Finger::State::Contact && touch[1].state != Finger::State::Contact)
	{
		if (offset > 0)
			offset = 0;
	}
}
