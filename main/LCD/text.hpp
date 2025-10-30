#pragma once

#include "color.hpp"
#include "vector.hpp"
#include "element.hpp"
#include "font.hpp"
#include "cmath"

template<ColorTemplate Color, Vector2us Size>
class Character final : public Element<Color, Size>
{
public:
	Vector2s position{};
	char text{};
	const Font* font = &fontBuiltIn;
	Color textColor{};
	Color backgroundColor{};
	unsigned char scale = 1;

	Character() = default;
	Character(Vector2s position, char text = '\0', Color textColor = Color::White, Color backgroundColor = Color::Black, unsigned char scale = 1, const Font* font = &fontBuiltIn) : position{ position }, text{ text }, font{ font }, textColor{ textColor }, backgroundColor{ backgroundColor }, scale{ scale } {}
	Character(Character&) = default;
	Character& operator=(Character&) = default;
	Character(Character&&) = default;
	Character& operator=(Character&&) = default;

	Vector2us getSize()
	{
		return font->getSize() * scale;
	}

	virtual bool isClicked(Vector2s point) override final
	{
		auto sub = point - position;
		return sub.x < font->getSize().x * scale &&
			sub.y < font->getSize().y * scale;
	}

	virtual Vector2us drawTo(Drawable<Color, Size>::DrawTarget& target, Vector2s offset = {}) override final
	{
		Vector2s fontSize = font->getSize();
		Vector2s modPositionStart = { 0,0 };
		Vector2s modPositionEnd = fontSize;

		Vector2s drawPosition = position + offset;
		if (drawPosition.x < 0)
		{
			modPositionStart.x = ((-drawPosition.x) + scale - 1) / scale;
			drawPosition.x = drawPosition.x % scale;
			if (drawPosition.x < 0) drawPosition.x += scale;
		}
		if (drawPosition.y < 0)
		{
			modPositionStart.y = ((-drawPosition.y) + scale - 1) / scale;
			drawPosition.y = drawPosition.y % scale;
			if (drawPosition.y < 0) drawPosition.y += scale;
		}
		if (drawPosition.x >= Size.x - fontSize.x * scale)
		{
			auto overflowPosition = drawPosition.x + fontSize.x * scale - Size.x + 1;
			modPositionEnd.x -= (overflowPosition + scale - 1) / scale;
		}
		if (drawPosition.y >= Size.y - fontSize.y * scale)
		{
			auto overflowPosition = drawPosition.y + fontSize.y * scale - Size.y + 1;
			modPositionEnd.y -= (overflowPosition + scale - 1) / scale;
		}
		Vector2s nowPosition = drawPosition;
		Vector2s repeatPosition = { 0,0 };
		const unsigned char* font = this->font->get(&text);

		for (auto y = modPositionStart.y; y < modPositionEnd.y; y++)
		{
			for (auto x = modPositionStart.x; x < modPositionEnd.x; x++)
			{
				const unsigned char& mod = font[(y * ((fontSize.x + 7) / 8)) + x / 8];
				bool draw = mod & ((1 << 7) >> (x % 8));
				Color& color = draw ? textColor : backgroundColor;

				for (repeatPosition.y = 0; repeatPosition.y < scale; repeatPosition.y++) for (repeatPosition.x = 0; repeatPosition.x < scale; repeatPosition.x++)
					target[nowPosition + repeatPosition] = color;
				nowPosition.x += scale;
			}
			nowPosition.x = drawPosition.x;
			nowPosition.y += scale;
		}

		return fontSize * scale;
	}
};

template<ColorTemplate Color, Vector2us Size>
class Text final : public Element<Color, Size>
{
public:
	Vector2s position{};
	Vector2s endPosition{};
	const char* text = "";
	const Font* font = fontBuiltIn;
	Color textColor{};
	Color backgroundColor{};
	unsigned char scale = 1;

	Text() = default;
	Text(Vector2s position, const char* text = "", Color textColor = Color::White, Color backgroundColor = Color::Black, unsigned char scale = 1, const Font* font = fontBuiltIn) : position{ position }, endPosition{ position }, text{ text }, font{ font }, textColor{ textColor }, backgroundColor{ backgroundColor }, scale{ scale } {}
	Text(Vector2s position, const char* text, unsigned char scale, Color textColor = Color::White, Color backgroundColor = Color::Black, const Font* font = fontBuiltIn) : position{ position }, endPosition{ position }, text{ text }, font{ font }, textColor{ textColor }, backgroundColor{ backgroundColor }, scale{ scale } {}
	Text(Text&) = default;
	Text& operator=(Text&) = default;
	Text(Text&&) = default;
	Text& operator=(Text&&) = default;

	Vector2us getSize()
	{
		return endPosition - position;
	}

	Vector2us computeSize()
	{
		Vector2s fontSize = font->getSize();

		endPosition = position;
		Vector2s nowPosition = position;

		for (const char* textPointer = text; *textPointer != '\0'; textPointer++)
		{
			switch (*textPointer)
			{
			case '\n':
				if (endPosition.x < nowPosition.x)
					endPosition.x = nowPosition.x;
				nowPosition.x = position.x;
				nowPosition.y += fontSize.y * scale;
				continue;
			case '\r':
				nowPosition.x = position.x;
				continue;
			case '\b': // 直接赌不会向左
				nowPosition.x -= fontSize.x * scale;
				continue;
			default:
				nowPosition.x += fontSize.x * scale;
			}
		}

		if (endPosition.x < nowPosition.x)
			endPosition.x = nowPosition.x;
		endPosition.y = nowPosition.y + fontSize.y * scale;

		return getSize();
	}

	virtual bool isClicked(Vector2s point) override final
	{
		return position.x <= point.x && point.x < endPosition.x &&
			position.y <= point.y && point.y < endPosition.y;
	}

	virtual Vector2us drawTo(Drawable<Color, Size>::DrawTarget& target, Vector2s offset = {}) override final
	{
		Vector2s fontSize = font->getSize();
		Vector2s drawPosition = position + offset;
		Character<Color, Size> tempCharacter{ drawPosition, '\0', textColor, backgroundColor,scale, font };
		Vector2s& nowPosition = tempCharacter.position;

		for (const char* textPointer = text; *textPointer != '\0'; textPointer++)
		{
			switch (*textPointer)
			{
			case '\n':
				nowPosition.x = drawPosition.x;
				nowPosition.y += fontSize.y * scale;
				continue;
			case '\r':
				nowPosition.x = drawPosition.x;
				continue;
			case '\b':
				nowPosition.x -= fontSize.x * scale;
				continue;
			case '\t':
				nowPosition.x += fontSize.x * scale;
				continue;
			}
			tempCharacter.text = *textPointer;
			nowPosition.x += tempCharacter.drawTo(target).x;
		}

		return nowPosition - drawPosition + Vector2s{ 0, fontSize.y } *scale;
	}
};

template<ColorTemplate Color, Vector2us Size, class T>
class Number final : public Element<Color, Size>
{
public:
	Vector2s position{};
	Vector2s endPosition{};
	T number{};
	const Font* font = fontBuiltIn;
	unsigned char base = 10;
	Color textColor{};
	Color backgroundColor{};
	unsigned char scale = 1;

	Number() = default;
	Number(Vector2s position, T number = T{}, unsigned char base = 10, Color textColor = Color::White, Color backgroundColor = Color::Black, unsigned char scale = 1, const Font* font = fontBuiltIn) : position{ position }, endPosition{ position }, number{ number }, font{ font }, base{ base }, textColor{ textColor }, backgroundColor{ backgroundColor }, scale{ scale } {}
	Number(Number&) = default;
	Number& operator=(Number&) = default;
	Number(Number&&) = default;
	Number& operator=(Number&&) = default;

	Vector2us getSize()
	{
		return endPosition - position;
	}

	Vector2us computeSize()
	{
		T nowNumber = number;
		endPosition = position;
		endPosition += Vector2s{ 8,16 } *scale;

		while (nowNumber > base)
		{
			nowNumber /= base;
			endPosition.x += 8 * scale;
		}

		return getSize();
	}

	virtual bool isClicked(Vector2s point) override final
	{
		return position.x <= point.x && point.x < endPosition.x &&
			position.y <= point.y && point.y < endPosition.y;
	}

	virtual Vector2us drawTo(Drawable<Color, Size>::DrawTarget& target, Vector2s offset = {}) override final
	{
		Vector2s drawPosition = position;
		Character<Color, Size> tempCharacter{ drawPosition, '0', textColor, backgroundColor, scale, font };
		Vector2s& nowPosition = tempCharacter.position;
		T nowNumber = number;

		if (nowNumber == 0)
			return tempCharacter.drawTo(target);

		if (nowNumber < 0)
		{
			tempCharacter.text = '-';
			nowPosition.x += tempCharacter.drawTo(target).x;
			nowNumber = -nowNumber;
		}

		auto draw = [this, &tempCharacter, &target](T number, auto draw)->void
			{
				constexpr char HexNumber[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };

				if (number >= this->base)
					draw(number / this->base, draw);

				tempCharacter.text = HexNumber[number % this->base];
				tempCharacter.position.x += tempCharacter.drawTo(target).x;
			};

		draw(nowNumber, draw); // 递归lambda

		return nowPosition - drawPosition + Vector2us{ 0,font->getSize().y } *scale;
	}
};
