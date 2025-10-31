#include <iostream>
#include <algorithm>
#include <cstring>
#include <fstream>

#include "gbk.hpp"

bool space3000ed = false;

int main(int argc, char* argv[])
{
	std::sort(gbkCharacter, gbkCharacter + sizeof(gbkCharacter) / sizeof(gbkCharacter[0]), [](Character a, Character b) -> bool { return (Unicode)a < (Unicode)b; });

	std::ofstream fontFile{ "build/out.font" };

	std::cout << std::showbase << std::uppercase << std::hex << "start\n";

	for (auto i : gbkCharacter)
	{
		if (i.unicode == 0XF7FD) continue; // 四字节
		if (0xE000 <= i.unicode && i.unicode <= 0xF8FF) continue; // 私有平面
		if (i.unicode == 0x3000) // 重复的空格
		{
			if (space3000ed) continue;
			space3000ed = true;
		}

		Utf8 out{ i.unicode };
		std::cout << i.unicode << '\'' << out[0] << out[1] << out[2] << "'\t";
	}

	for (auto i : gbkCharacter)
	{
		if (i.unicode == 0XF7FD) continue; // 四字节
		if (0xE000 <= i.unicode && i.unicode <= 0xF8FF) continue; // 私有平面
		if (i.unicode == 0x3000) // 重复的空格
		{
			if (space3000ed) continue;
			space3000ed = true;
		}
		fontFile.write((const char*)&i.unicode, sizeof(i.unicode));
	}

	for (auto i : gbkCharacter)
	{
		if (i.unicode == 0XF7FD) continue; // 四字节
		if (0xE000 <= i.unicode && i.unicode <= 0xF8FF) continue; // 私有平面
		if (i.unicode == 0x3000) // 重复的空格
		{
			if (space3000ed) continue;
			space3000ed = true;
		}
		fontFile.write((const char*)&i.font, sizeof(i.font));
	}

	return 0;
}
