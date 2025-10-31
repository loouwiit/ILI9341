#pragma once

using Unicode = unsigned short;
constexpr Unicode UnicodeError = 0xFFFD;

class Utf8
{
public:
	constexpr static Unicode Byte1MaxInUnicode = 0x007F;
	constexpr static Unicode Byte2MaxInUnicode = 0x07FF;
	constexpr static Unicode Byte3MaxInUnicode = 0xFFFF;

	constexpr static unsigned getUft8LengthFromUnicode(Unicode unicode)
	{
		return 1 +
			(unicode > Byte1MaxInUnicode) +
			(unicode > Byte2MaxInUnicode) +
			(unicode > Byte3MaxInUnicode);
	}

	constexpr Utf8(Unicode unicode)
	{
		text[0] = 0xE0 | ((unicode >> 12) & 0x0F);// 0b1110xxxx
		text[1] = 0x80 | ((unicode >> 5) & 0x3F); // 0b10xxxxxx
		text[2] = 0x80 | ((unicode >> 0) & 0x3F); // 0b10xxxxxx
	}

	constexpr Utf8(const char text[3])
	{
		if ((text[0] & 0xF0) != 0xE0) [[unlikely]] goto detect1Byte; // 0b1110xxxx
		if ((text[1] & 0xC0) != 0x80) [[unlikely]] goto detect1Byte; // 0b10xxxxxx
		if ((text[2] & 0xC0) != 0x80) [[unlikely]] goto detect1Byte; // 0b10xxxxxx
		this->text[0] = text[0];
		this->text[1] = text[1];
		this->text[2] = text[2];
		return;

	detect1Byte:
		if ((text[0] & 0x80) == 0x00) [[unlikely]] goto detect2Byte;
		this->text[0] = text[0];
		this->text[1] = 0x00;
		this->text[2] = 0x00;
		return;

	detect2Byte:
		if ((text[0] & 0xE0) != 0xC0) [[likely]] goto error; // 0b110xxxxx
		if ((text[1] & 0xC0) != 0x80) [[likely]] goto error; // 0b10xxxxxx
		this->text[0] = text[0];
		this->text[1] = text[1];
		this->text[2] = 0x00;
		return;

	error:
		this->text[0] = 0xEF;
		this->text[1] = 0xBF;
		this->text[2] = 0xBD;
		return;
	}

	constexpr unsigned char operator[](int index) const { return text[index]; }

	constexpr Unicode getUnicode() const
	{
		if (text[2] != 0x00) [[likely]]
			return // 3 byte
			(((unsigned short)text[0] & 0x0F) << 12) |
			(((unsigned short)text[1] & 0x3F) << 6) |
			(((unsigned short)text[2] & 0x3F) << 0);
		else if (text[1] != 0x00) [[unlikely]]
			return // 2 byte
			(((unsigned short)text[0] & 0x1F) << 6) |
			(((unsigned short)text[1] & 0x3F) << 0);
		else return text[0]; // 1 byte
	}

	constexpr operator Unicode() const { return getUnicode(); }

private:
	unsigned char text[3]{};
};
