#include <iostream>
#include <fstream>
#include <unistd.h>
#include <format>
#include <cstring>

#include "socketStream.hpp"

#include <sys/socket.h>
#include <arpa/inet.h>

char buffer[5000] = "";
constexpr unsigned short defaultPort = 467;
unsigned short port = defaultPort;

class CpuStat
{
public:
	void update(std::istream& file)
	{
		unsigned user = 0;
		unsigned root = 0;
		unsigned system = 0;
		unsigned idle = 0;
		unsigned io = 0;

		file.clear();
		file.seekg(0);
		std::string garbage;
		file >> garbage;
		file >> user >> root >> system >> idle >> io;

		auto all = user + root + system + idle + io
			- userLast - rootLast - systemLast - idleLast - ioLast;
		userPresent = (user - userLast) * 100 / all;
		rootPresent = (root - rootLast) * 100 / all;
		systemPresent = (system - systemLast) * 100 / all;
		idlePresent = (idle - idleLast) * 100 / all;
		ioPresent = (io - ioLast) * 100 / all;

		userLast = user;
		rootLast = root;
		systemLast = system;
		idleLast = idle;
		ioLast = io;
	}

	unsigned userPresent = 0;
	unsigned rootPresent = 0;
	unsigned systemPresent = 0;
	unsigned idlePresent = 0;
	unsigned ioPresent = 0;

	unsigned userLast = 0;
	unsigned rootLast = 0;
	unsigned systemLast = 0;
	unsigned idleLast = 0;
	unsigned ioLast = 0;
};

CpuStat cpuStat{};

int read(std::istream& file)
{
	int ret = 0;
	file.clear();
	file.seekg(0);
	file >> ret;
	return ret;
}

int main(int argc, char* argv[])
{
	std::ifstream gpuUsage{ "/sys/class/drm/card1/device/gpu_busy_percent" };
	if (!gpuUsage.is_open())
	{
		std::cerr << "error to open gpu usage";
		return -1;
	}

	std::ifstream gpuPower{ "/sys/class/drm/card1/device/hwmon/hwmon8/power1_average" };
	if (!gpuPower.is_open())
	{
		std::cerr << "error to open gpu power";
		return -1;
	}

	std::ifstream gpuTempure{ "/sys/class/drm/card1/device/hwmon/hwmon8/temp1_input" };
	if (!gpuTempure.is_open())
	{
		std::cerr << "error to open gpu power";
		return -1;
	}

	std::ifstream cpu{ "/proc/stat" };
	if (!cpu.is_open())
	{
		std::cerr << "error to open cpu stat";
		return -1;
	}

	std::string ip = "";
	if (argc >= 2)
	{
		ip = argv[1];
		std::cout << "ip:" << ip << '\n';
	}
	else
	{
		std::cout << "ip:";
		std::cin >> ip;
	}

	if (argc >= 3)
	{
		port = atoi(argv[2]);
		if (port == 0) port = defaultPort;
		std::cout << "port:" << port << '\n';
	}
	else
	{
		std::cout << "port:";
		std::cin >> port;
		if (port == 0) port = defaultPort;
	}

	Socket sock = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in serv_addr{};
	memset(&serv_addr, 0, sizeof(serv_addr));  //每个字节都用0填充
	serv_addr.sin_family = AF_INET;  //使用IPv4地址
	serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());  //具体的IP地址
	serv_addr.sin_port = htons(port);  //端口

	if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)))
	{
		std::cout << "connect failed\n";
		return -1;
	}
	else std::cout << "connected\n";

	IOSocketStream socketStream{};
	socketStream.setSocket(sock);

	int size = 0;
	while (socketStream.isGood())
	{
		size = sprintf(buffer, "gpu:%d%%\ntemp:%d'C\npower:%dw\n", read(gpuUsage), read(gpuTempure) / 1000, read(gpuPower) / 1000000);
		if (size > 0)
		{
			std::cout << buffer;
			socketStream.write(buffer, size);
		}
		cpuStat.update(cpu);
		size = sprintf(buffer, "cpu:%d%%\n", 100 - cpuStat.idlePresent);
		if (size > 0)
		{
			std::cout << buffer;
			socketStream.write(buffer, size);
		}
		socketStream.put('\0');
		socketStream.sendNow();
		sleep(1);
	}

	return 0;
}