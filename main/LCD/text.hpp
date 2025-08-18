#pragma once

#include "color.hpp"
#include "vector.hpp"
#include "drawable.hpp"
#include "font.hpp"
#include "cmath"

template<ColorTemplate Color, Vector2us Size>
class Character : public Drawable<Color, Size>
{
public:
	Vector2us position{};
	char text{};
	Color textColor{};
	Color backgroundColor{};
	unsigned char scale = 1;

	Character() = default;
	Character(Vector2us position, char text = '\0', Color textColor = Color::White, Color backgroundColor = Color::Black, unsigned char scale = 1) : position{ position }, text{ text }, textColor{ textColor }, backgroundColor{ backgroundColor }, scale{ scale } {}
	Character(Character&) = default;
	Character& operator=(Character&) = default;
	Character(Character&&) = default;
	Character& operator=(Character&&) = default;

	virtual Vector2us drawTo(Drawable<Color, Size>::DrawTarget& target) override
	{
		if (text < 0x20) return { 0,0 };

		Vector2us nowPosition = position;
		Vector2us repeatPosition = { 0,0 };
		const unsigned char* font = fonts[(unsigned char)text - 0x20];

		for (unsigned char y = 0; y < 16; y++)
		{
			const unsigned char& mod = font[y];
			for (unsigned char x = 0; x < 8; x++)
			{
				bool draw = mod & ((1 << 7) >> x);
				Color& color = draw ? textColor : backgroundColor;

				for (repeatPosition.y = 0; repeatPosition.y < scale; repeatPosition.y++) for (repeatPosition.x = 0; repeatPosition.x < scale; repeatPosition.x++)
					target[nowPosition + repeatPosition] = color;
				nowPosition.x += scale;
			}
			nowPosition.x -= 8 * scale;
			nowPosition.y += scale;
		}

		return Vector2us{ 8,16 } *scale;
	}
};

template<ColorTemplate Color, Vector2us Size>
class Text : public Drawable<Color, Size>
{
public:
	Vector2us position{};
	const char* text = "";
	Color textColor{};
	Color backgroundColor{};
	unsigned char scale = 1;

	Text() = default;
	Text(Vector2us position, const char* text = "", Color textColor = Color::White, Color backgroundColor = Color::Black, unsigned char scale = 1) : position{ position }, text{ text }, textColor{ textColor }, backgroundColor{ backgroundColor }, scale{ scale } {}
	Text(Text&) = default;
	Text& operator=(Text&) = default;
	Text(Text&&) = default;
	Text& operator=(Text&&) = default;

	virtual Vector2us drawTo(Drawable<Color, Size>::DrawTarget& target) override
	{
		Vector2us lineBegin = position;
		Character<Color, Size> tempCharacter{ position, '\0', textColor, backgroundColor,scale };
		Vector2us& nowPosition = tempCharacter.position;

		for (const char* textPointer = text; *textPointer != '\0'; textPointer++)
		{
			switch (*textPointer)
			{
			case '\n':
				lineBegin += Vector2us{ 0, 16 } *scale;
				nowPosition = lineBegin;
				continue;
			case '\r':
				nowPosition = lineBegin;
				continue;
			case '\b':
				nowPosition.x -= 8 * scale;
				continue;
			case '\t':
				nowPosition.x += 8 * scale;
				continue;
			}
			tempCharacter.text = *textPointer;
			nowPosition.x += tempCharacter.drawTo(target).x;
		}

		return nowPosition - position + Vector2us{ 0, 16 } *scale;
	}
};

template<ColorTemplate Color, Vector2us Size, class T>
class Number : public Drawable<Color, Size>
{
public:
	Vector2us position{};
	T number{};
	unsigned char base = 10;
	Color textColor{};
	Color backgroundColor{};
	unsigned char scale = 1;

	Number() = default;
	Number(Vector2us position, T number = T{}, unsigned char base = 10, Color textColor = Color::White, Color backgroundColor = Color::Black, unsigned char scale = 1) : position{ position }, number{ number }, base{ base }, textColor{ textColor }, backgroundColor{ backgroundColor }, scale{ scale } {}
	Number(Number&) = default;
	Number& operator=(Number&) = default;
	Number(Number&&) = default;
	Number& operator=(Number&&) = default;

	virtual Vector2us drawTo(Drawable<Color, Size>::DrawTarget& target) override
	{
		Character<Color, Size> tempCharacter{ position, '0', textColor, backgroundColor, scale };
		Vector2us& nowPosition = tempCharacter.position;
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

		return nowPosition - position + Vector2us{ 0,16 } *scale;
	}
};
