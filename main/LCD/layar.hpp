#pragma once

#include "color.hpp"
#include "vector.hpp"
#include "element.hpp"
#include "algorithm"

namespace LayarClassicSize
{
	constexpr unsigned char Pair = 2;
	constexpr unsigned char Small = 5;
	constexpr unsigned char Middle = 10;
};

template <ColorTemplate Color, Vector2us Size, unsigned char elementMaxSize>
class Layar : public Element<Color, Size>
{
public:
	Vector2us start{};
	Vector2us end{};

	unsigned char elementCount = 0;
	Element<Color, Size>* elements[elementMaxSize];

	Layar(Layar&) = default;
	Layar& operator=(Layar&) = default;
	Layar(Layar&& move) = default;
	Layar& operator=(Layar&& move) = default;

	Layar(unsigned char elementCount = elementMaxSize) : elementCount{ elementCount } { assert(elementCount <= elementMaxSize); }

	Layar(Vector2us start, Vector2us size, unsigned char elementCount = elementMaxSize) : start{ start }, end{ start + size }, elementCount{ elementCount } { assert(elementCount <= elementMaxSize); }

	auto& operator[](unsigned char index) { return elements[index]; }

	Vector2us getSize()
	{
		return end - start;
	}

	virtual bool isClicked(Vector2us point) override final
	{
		return start.x <= point.x && point.x < end.x &&
			start.y <= point.y && point.y < end.y;
	}

	virtual void finger(Finger& finger) override final
	{
		for (unsigned char i = 0; i < elementCount; i++)
			elements[i]->finger(finger);
	}

	virtual Vector2us drawTo(Drawable<Color, Size>::DrawTarget& target) override
	{
		for (unsigned char i = 0; i < elementCount; i++)
			elements[i]->drawTo(target);
		return end - start;
	}
};
