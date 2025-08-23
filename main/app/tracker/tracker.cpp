#include "tracker.hpp"

#include <lwip/sockets.h>

#include "wifi/wifi.hpp"

constexpr static char TAG[] = "tracker";

void AppTracker::init()
{
	App::init();
	xTaskCreate(serverThread, "appTracker", 4096, this, 2, nullptr);
}

void AppTracker::deinit()
{
	running = false;
}

void AppTracker::draw()
{
	lcd.clear();
	lcd.draw(text);
}

void AppTracker::touchUpdate()
{
	if ((touch[0].state == Finger::State::Press && touch[1].state == Finger::State::Contact) ||
		(touch[1].state == Finger::State::Press && touch[0].state == Finger::State::Contact) ||
		(touch[0].state == Finger::State::Contact && touch[1].state == Finger::State::Contact))
		changeAppCallback(nullptr);

	if (touch[0].state == Finger::State::Press)
	{
		fingerActive[0] = true;
		lastFingerPosition[0] = touch[0].position;
	}
	else if (touch[0].state == Finger::State::Realease)
		fingerActive[0] = false;

	if (touch[1].state == Finger::State::Press)
	{
		fingerActive[1] = true;
		lastFingerPosition[1] = touch[1].position;
	}
	else if (touch[1].state == Finger::State::Realease)
		fingerActive[1] = false;

	if (fingerActive[0])
	{
		text.position += touch[0].position - lastFingerPosition[0];
		lastFingerPosition[0] = touch[0].position;
	}
	if (fingerActive[1])
	{
		text.position += touch[1].position - lastFingerPosition[1];
		lastFingerPosition[1] = touch[1].position;
	}
}

void AppTracker::back()
{
	changeAppCallback(nullptr);
}

void AppTracker::update(const char* string)
{
	strcpy(buffer, string);
}

void AppTracker::dealSocket(IOSocketStream& socketStream)
{
	unsigned dealIndex = 0;
	unsigned recieveIndex = 0;
	char* netBuffer = (char*)heap_caps_malloc(1024, MALLOC_CAP_SPIRAM);

	constexpr static clock_t TimeOut = 5000;
	clock_t nextTime = TimeOut + clock();

	while (running && socketStream.isGood())
	{
		while (!socketStream.check() && socketStream.isGood())
		{
			if (nextTime < clock())
			{
				socketStream.close();
				break;
			}
			vTaskDelay(1);
		}
		nextTime = TimeOut + clock();

		recieveIndex += socketStream.readByte(netBuffer + recieveIndex, sizeof(buffer) - recieveIndex);

		for (; dealIndex < recieveIndex; dealIndex++) if (netBuffer[dealIndex] == '\0')
		{
			strcpy(buffer, netBuffer);
			dealIndex++; // skip for '\0'
			memcpy(netBuffer, netBuffer + dealIndex, recieveIndex - dealIndex);
			recieveIndex -= dealIndex;
			dealIndex = 0;
			break;
		}
	}
	free(netBuffer);
	netBuffer = nullptr;
	socketStream.close();
}

void AppTracker::serverThread(void* param)
{
	AppTracker& self = *(AppTracker*)param;

	if (!wifiIsInited())
	{
		ESP_LOGW(TAG, "wifi not inited");
		self.update("wifi not inited");
		self.deleteAble = true;
		vTaskDelete(nullptr);
	}

	int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (listen_sock < 0)
	{
		ESP_LOGE(TAG, "socket creat failed: error %d", listen_sock);
		self.update("socket creat failed");
		close(listen_sock);
		self.deleteAble = true;
		vTaskDelete(nullptr);
	}

	int opt = 1;
	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	sockaddr_storage addr = {};
	{
		struct sockaddr_in* dest_addr_ip4 = (struct sockaddr_in*)&addr;
		dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
		dest_addr_ip4->sin_family = AF_INET;
		dest_addr_ip4->sin_port = htons(Port);
	}

	int err = bind(listen_sock, (struct sockaddr*)&addr, sizeof(addr));
	if (err != 0)
	{
		ESP_LOGE(TAG, "socket bind failed: errno %d\n", errno);
		self.update("socket bind failed");
		close(listen_sock);
		self.deleteAble = true;
		vTaskDelete(nullptr);
	}

	err = listen(listen_sock, 1);
	if (err != 0)
	{
		ESP_LOGE(TAG, "socket listen failed: errno %d\n", errno);
		self.update("socket listen failed");
		close(listen_sock);
		self.deleteAble = true;
		vTaskDelete(nullptr);
		return;
	}

	while (self.running)
	{
		ESP_LOGI(TAG, "socket listening\n");
		self.update("wait for connection");

		struct sockaddr_storage source_addr;
		socklen_t addr_len = sizeof(source_addr);
		int sock = accept(listen_sock, (struct sockaddr*)&source_addr, &addr_len);
		if (sock < 0)
		{
			ESP_LOGE(TAG, "Unable to accept connection: errno %d\n", errno);
			self.update("Unable to accept connection");
			break;
		}

		// Set tcp keepalive option
		int keepAlive = 1;
		int keepIdle = 5;
		int keepInterval = 5;
		int keepCount = 3;
		setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
		setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
		setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
		setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));

		// Convert ip address to string
		char addr_str[128];
		if (source_addr.ss_family == PF_INET)
		{
			inet_ntoa_r(((sockaddr_in*)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
		}

		ESP_LOGI(TAG, "socket accepted ip address: %s\n", addr_str);

		IOSocketStream socketStream;
		socketStream.setSocket(sock);
		self.dealSocket(socketStream);
		self.update("disconnected");
	}

	close(listen_sock);
	self.deleteAble = true;
	vTaskDelete(nullptr);
}
