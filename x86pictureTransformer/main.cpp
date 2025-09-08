#include <iostream>
#include <fstream>

#include <SFML/Graphics.hpp>

#include "color.hpp"
#include "yuv.hpp"

constexpr unsigned short TotolFrames = 600;

using Color = Color565;
unsigned char scale = 1;
sf::Vector2i ScreenSize = { 320 / scale, 240 / scale };
bool rgb3111Enabled = false;

int appendRgb(std::ofstream& file, sf::Image& image);
int appendYuv420(std::ofstream& file, sf::Image& image);

int main(int argc, char* argv[])
{
	std::string path;

	std::cout << "scale(+):";
	scale = std::min(std::max(1, std::cin.get() - '0'), 16);
	if (std::cin.peek() == '+')
	{
		rgb3111Enabled = true;
		std::cin.ignore();
	}

	if (std::cin.peek() == ' ' ||
		std::cin.peek() == '\n')
		std::cin.ignore();

	ScreenSize = { 320 / scale, 240 / scale };

	std::cout << "path:";
	std::getline(std::cin, path, '\n');

	std::string command{};
	if (path[0] == '"' || path[0] == '\'')
		command = "ffmpeg -i " + path;
	else
		command = "ffmpeg -i \"" + path + "\"";

	command += " -t 60s -r 10 -f image2 -vf scale=";
	command += std::to_string(ScreenSize.x);
	command += ':';
	command += std::to_string(ScreenSize.y);
	command += " files/%05d.png";

	system(command.c_str());

	std::ofstream file{ "files/out.pic" };
	sf::Image image;
	image.create(ScreenSize.x, ScreenSize.y);

	char totolCount[2] = { 0x00,0x00 };
	file.write(&totolCount[0], 2);

	scale |= rgb3111Enabled << 7;
	file.write((const char*)&scale, 1);
	scale &= ~(1 << 7);

	unsigned short pictureIndex = 1;
	for (; pictureIndex <= TotolFrames; pictureIndex++)
	{
		char buffer[] = "files/00000.png";
		sprintf(buffer, "files/%05d.png", pictureIndex);
		if (image.loadFromFile(buffer))
		{
			if (rgb3111Enabled)
				appendYuv420(file, image);
			else
				appendRgb(file, image);
		}
		else break;
	}

	pictureIndex--;
	file.seekp(0);
	file.put(pictureIndex & 0xFF);
	file.put(pictureIndex >> 8);

	std::cout << pictureIndex << "frame totol\n";

	return 0;
}

int appendRgb(std::ofstream& file, sf::Image& image)
{
	sf::Color colorInput{};
	Color colorOutput{};
	uint16_t fileOutput{};

	for (int y = 0; y < ScreenSize.y; y++) for (int x = 0; x < ScreenSize.x; x++)
	{
		colorInput = image.getPixel(x, y);
		colorInput.r >>= 2;
		colorInput.g >>= 2;
		colorInput.b >>= 2;

		colorOutput = Color{ colorInput.r, colorInput.g,colorInput.b };

		fileOutput = (uint16_t)colorOutput;
		file.put(fileOutput >> 8);
		file.put(fileOutput & 0xFF);
	}

	return 0;
}

int appendYuv420(std::ofstream& file, sf::Image& image)
{
	sf::Color colorInput[4]{};
	Color color[4]{};
	bool yExtra = false;
	YUV yuv[4]{};
	uint16_t fileOutput{};

	Color reverse{};

	for (int y = 0; y < ScreenSize.y; y += 2) for (int x = 0; x < ScreenSize.x; x += 2)
	{
		colorInput[0] = image.getPixel(x + 0, y + 0);
		colorInput[1] = image.getPixel(x + 0, y + 1);
		colorInput[2] = image.getPixel(x + 1, y + 0);
		colorInput[3] = image.getPixel(x + 1, y + 1);

		for (int i = 0; i < 4; i++)
			color[i] = Color{ (uint8_t)(colorInput[i].r >> 2), (uint8_t)(colorInput[i].g >> 2), (uint8_t)(colorInput[i].b >> 2) };

		for (int i = 0; i < 4; i++)
			yuv[i] = color[i];

		yExtra = yuv[0].y & (1 << 2);
		for (int i = 0; i < 4; i++)
			yuv[i].y = (yuv[i].y & 0xF8) + yExtra * 0x07; // 模拟并补偿存储时的损失

		yuv[0].u = yuv[0].u < 0 ? (yuv[0].u + 7) & 0xF8 : yuv[0].u & 0xF8;
		yuv[0].v = yuv[0].v < 0 ? (yuv[0].v + 7) & 0xF8 : yuv[0].v & 0xF8;

		for (int i = 1; i < 4; i++)
		{
			yuv[i].u = yuv[0].u;
			yuv[i].v = yuv[0].v;
		}

		for (int i = 1; i < 4;i++)
		{
			reverse = (Color)yuv[i];
			while (reverse.getG() < 5 && color[i].getG() > 28)
			{
				// G溢出，减少Y
				yuv[i].y -= 1 << 3;
				reverse = (Color)yuv[i];
			}
		}

		reverse = yuv[0]; // for debug
		reverse = yuv[1]; // for debug
		reverse = yuv[2]; // for debug
		reverse = yuv[3]; // for debug

		yuv[1].y -= yuv[0].y & 0xF8;
		yuv[2].y -= yuv[0].y & 0xF8;
		yuv[3].y -= yuv[0].y & 0xF8;

		fileOutput = (yExtra << 15) +
			((yuv[0].y >> 3) << 10) +
			(((yuv[0].u + 0x7F) >> 3) << 5) +
			(((yuv[0].v + 0x7F) >> 3) << 0);
		file.put(fileOutput >> 8);
		file.put(fileOutput & 0xFF);

		fileOutput = ((yuv[1].y >> 3) << 10) +
			((yuv[2].y >> 3) << 5) +
			((yuv[3].y >> 3) << 0);
		file.put(fileOutput >> 8);
		file.put(fileOutput & 0xFF);
	}

	return 0;
}
