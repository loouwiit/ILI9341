#pragma once

#include "drawable.hpp"
#include "vector.hpp"
#include "color.hpp"
#include "frame.hpp"
#include "font.hpp"
#include "cmath"

template <ColorTemplate Color>
class Character : public Drawable<Character<Color>>
{
public:
	Vector2us position{};
	char text{};
	Color textColor{};
	Color backgroundColor{};

	Character() = default;
	Character(Vector2us position, char text, Color textColor = Color::White, Color backgroundColor = Color::Black) : position{ position }, text{ text }, textColor{ textColor }, backgroundColor{ backgroundColor } {}
	Character(Character&) = default;
	Character& operator=(Character&) = default;
	Character(Character&&) = default;
	Character& operator=(Character&&) = default;

	Vector2us drawTo(auto& target) const
	{
		if (text < 0x20) return { 0,0 };

		Vector2us nowPosition = position;
		const unsigned char* font = fonts[(unsigned char)text - 0x20];

		for (unsigned char i = 0;i < 16;i++)
		{
			const unsigned char& mod = font[i];
			for (unsigned char j = 0; j < 8; j++)
			{
				bool draw = mod & ((1 << 7) >> j);
				if (draw)
					target[nowPosition] = textColor;
				else
					target[nowPosition] = backgroundColor;

				nowPosition.x++;
			}
			nowPosition.x -= 8;
			nowPosition.y++;
		}

		return { 8,16 };
	}
};

template <ColorTemplate Color>
class Text : public Drawable<Text<Color>>
{
public:
	Vector2us position{};
	const char* text = "";
	Color textColor{};
	Color backgroundColor{};

	Text() = default;
	Text(Vector2us position, const char* text, Color textColor = Color::White, Color backgroundColor = Color::Black) : position{ position }, text{ text }, textColor{ textColor }, backgroundColor{ backgroundColor } {}
	Text(Text&) = default;
	Text& operator=(Text&) = default;
	Text(Text&&) = default;
	Text& operator=(Text&&) = default;

	Vector2us drawTo(auto& target) const
	{
		Vector2us lineBegin = position;
		Character<Color> tempCharacter{ position, '\0', textColor, backgroundColor };
		Vector2us& nowPosition = tempCharacter.position;

		for (const char* textPointer = text; *textPointer != '\0'; textPointer++)
		{
			switch (*textPointer)
			{
			case '\n':
				lineBegin += Vector2us{ 0, 16 };
				nowPosition = lineBegin;
				continue;
			case '\r':
				nowPosition = lineBegin;
				continue;
			case '\b':
				nowPosition.x -= 8;
				continue;
			case '\t':
				nowPosition.x += 8;
				continue;
			}
			tempCharacter.text = *textPointer;
			tempCharacter.drawTo(target);

			nowPosition.x += 8;
		}

		return nowPosition - position + Vector2us{ 0, 16 };
	}
};

template <ColorTemplate Color, class T>
class Number : public Drawable<Number<Color, T>>
{
public:
	Vector2us position;
	T number;
	unsigned char base;
	Color textColor;
	Color backgroundColor;

	Number() = default;
	Number(Vector2us position, T number, unsigned char base = 10, Color textColor = Color::White, Color backgroundColor = Color::Black) : position{ position }, number{ number }, base{ base }, textColor{ textColor }, backgroundColor{ backgroundColor } {}
	Number(Number&) = default;
	Number& operator=(Number&) = default;
	Number(Number&&) = default;
	Number& operator=(Number&&) = default;

	Vector2us drawTo(auto& target) const
	{
		Character<Color> tempCharacter{ position, '0', textColor, backgroundColor };
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

		return nowPosition - position + Vector2us{ 0,16 };
	}
};
