#pragma once

#include "vector.hpp"
#include "utf8.hpp"

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
extern const Font* fontBuiltIn;
extern const Font* fontChinese;
