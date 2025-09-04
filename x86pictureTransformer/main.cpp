#include <iostream>
#include <fstream>

#include <SFML/Graphics.hpp>

#include "color.hpp"

constexpr unsigned short TotolFrames = 600;

using Color = Color565;
unsigned char scale = 1;
sf::Vector2i ScreenSize = { 320 / scale, 240 / scale };

int appendPicture(std::ofstream& file, sf::Image& image);

int main(int argc, char* argv[])
{
	std::string path;
	if (argc > 1)
		scale = std::max(1, atoi(argv[1]));
	else
	{
		std::cout << "scale:";
		scale = std::max(1, std::cin.get() - '0');
		if (std::cin.peek() == ' ' ||
			std::cin.peek() == '\n')
			std::cin.ignore();
	}

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
	file.write((const char*)&scale, 1);

	unsigned short pictureIndex = 1;
	for (; pictureIndex <= TotolFrames; pictureIndex++)
	{
		char buffer[] = "files/00000.png";
		sprintf(buffer, "files/%05d.png", pictureIndex);
		if (image.loadFromFile(buffer))
		{
			appendPicture(file, image);
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

int appendPicture(std::ofstream& file, sf::Image& image)
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
