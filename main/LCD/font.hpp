#pragma once

#include "vector.hpp"

class Font
{
public:
	virtual const unsigned char* get(char* text) const { return error; };
	inline const unsigned char* operator()(char* text) const { return get(text); }

	Vector2us getSize() const { return textSize; }

protected:
	constexpr Font(Vector2us textSize = {}) : textSize{ textSize } {}
	Vector2us textSize{ 0,0 };

	constexpr static unsigned char error[16] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };
};

extern const Font* fontBuiltIn;
extern const Font* fontBuiltInEqualWidth;
