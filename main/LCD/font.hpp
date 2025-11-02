#pragma once

#include <initializer_list>

#include "vector.hpp"
#include "utf8.hpp"
#include <esp_log.h>

class Font
{
public:
	virtual ~Font() {};

	virtual const unsigned char* get(Unicode text) const { return error; };
	inline const unsigned char* operator()(Unicode text) const { return get(text); }

	Vector2us getSize() const { return textSize; }

	static Font* load(const char* path);

	static const unsigned char* const error;

protected:
	constexpr Font(Vector2us textSize = {}) : textSize{ textSize } {}
	Vector2us textSize{ 0,0 };

	constexpr static unsigned char errorTable[32] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };
};

extern const Font* const fontBuiltInHalfWidth;
extern const Font* const fontBuiltInFullWidth;
extern const Font* const fontNone;
extern const Font* fontBuiltIn;
extern const Font* fontChinese;

class Fonts
{
public:
	struct Character
	{
		Vector2us size{};
		const unsigned char* font = nullptr;
	};

	Fonts()
	{
		for (auto& i : fonts) i = fontNone;
	}

	Fonts(const std::initializer_list<const Font*> list)
	{
		int i = 0;
		for (; i < list.size() && i < MaxFontSize; i++)
			fonts[i] = list.begin()[i];
		for (; i < MaxFontSize; i++)
			fonts[i] = fontBuiltIn;

		for (i = 0; i < MaxFontSize; i++)
			fontSize[i] = fonts[i]->getSize();
	}

	Character get(Unicode text) const
	{
		const unsigned char* ret = Font::error;
		for (int i = 0; i < MaxFontSize; i++)
		{
			ret = fonts[i]->get(text);
			if (ret != Font::error) return { fontSize[i], ret };
		}
		return { {8,16}, Font::error };
	}
	inline Character operator()(Unicode text) const { return get(text); }

	const Font* operator[](int index) const { return fonts[index]; }
	void setFont(int index, const Font* font) { fonts[index] = font; fontSize[index] = font->getSize(); }

private:
	constexpr static int MaxFontSize = 3;

	const Font* fonts[MaxFontSize];
	Vector2us fontSize[MaxFontSize];
};

extern Fonts fontsDefault;
extern Fonts fontsFullWidth;
