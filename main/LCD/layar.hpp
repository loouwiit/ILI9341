#pragma once

#include "color.hpp"
#include "vector.hpp"
#include "drawable.hpp"
#include "algorithm"

template <ColorTemplate Color, Vector2us Size, unsigned char elementMaxSize>
class Layar : public Drawable<Color, Size>
{
public:
	Vector2us start{};
	Vector2us end{};

	unsigned char elementCount = 0;
	Drawable<Color, Size>* elements[elementMaxSize];

	Layar(Layar&) = default;
	Layar& operator=(Layar&) = default;
	Layar(Layar&& move) = default;
	Layar& operator=(Layar&& move) = default;

	Layar(unsigned char elementCount = elementMaxSize) : elementCount{ elementCount } {}

	Layar(Vector2us start, Vector2us end, unsigned char elementCount = elementMaxSize) : start{ start }, end{ end }, elementCount{ elementCount } {}

	auto& operator[](unsigned char index) { return elements[index]; }

	virtual Vector2us drawTo(Drawable<Color, Size>::DrawTarget& target) override
	{
		for (unsigned char i = 0; i < elementCount; i++)
			elements[i]->drawTo(target);
		return end - start + Vector2us{ 1,1 };
	}
};
