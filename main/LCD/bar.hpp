#pragma once

#include "element.hpp"

template<ColorTemplate Color, Vector2us Size, class T>
class Bar final : public Element<Color, Size>
{
public:
	Bar(Vector2s position = {}, short barLength = 100, short barHight = 20, short slideSize = 30, Color backGroundColor = Color::Black, Color slideColor = Color::White) :
		position{ position }, slideSize{ slideSize }
	{
		bar.start = { 0,(short)(-barHight / 2) };
		bar.end = { barLength,(short)(barHight / 2) };
		bar.color = backGroundColor;

		slide.start = Vector2s{ (short)(-slideSize / 2),(short)(-slideSize / 2) };
		slide.end = Vector2s{ (short)(slideSize / 2),(short)(slideSize / 2) };
		slide.color = slideColor;
	}

	virtual bool isClicked(Vector2s point) override final
	{
		point -= position;
		return bar.isClicked(point) || slide.isClicked(point);
	}

	virtual void finger(Finger finger) override final
	{
		finger.position -= position;
		switch (finger.state)
		{
		case Finger::State::None: break;
		case Finger::State::Press:
		{
			if (!(bar.isClicked(finger.position) || slide.isClicked(finger.position))) break;
			lastFingerPosition = finger.position;
			activeToMove = true;
			Clickable::pressCallback(finger, Clickable::clickCallbackParam);
			break;
		}
		case Finger::State::Realease:
		{
			if (!activeToMove) break;
			activeToMove = false;
			Clickable::releaseCallback(finger, Clickable::clickCallbackParam);
			break;
		}
		case Finger::State::Contact:
		{
			if (!activeToMove) break;
			if (finger.position == lastFingerPosition) break;

			auto diff = finger.position.x - lastFingerPosition.x;
			lastFingerPosition = finger.position;

			if (diff < -number)
				diff = -number;
			if (diff > bar.end.x - number)
				diff = bar.end.x - number;

			if (diff == 0) break;

			number += diff;
			slide.start.x += diff;
			slide.end.x += diff;

			Clickable::holdCallback(finger, Clickable::clickCallbackParam);
			break;
		}
		}
	}

	virtual Vector2us drawTo(Drawable<Color, Size>::DrawTarget& target, Vector2s offset = {}) override final
	{
		offset += position;
		bar.drawTo(target, offset);
		slide.drawTo(target, offset);
		return bar.end;
	}

	void setValue(T value)
	{
		if (value < 0) value = 0;
		if (value > bar.end.x) value = bar.end.x;
		number = value;
		slide.start.x = value - (slideSize >> 1);
		slide.end.x = value + (slideSize >> 1);
	}

	T getValue()
	{
		return number;
	}

	short getSlideSize()
	{
		return slideSize;
	}

	Color& barColor = bar.color;

	Vector2s position{};

private:
	Rectangle<Color, Size> bar{};
	short slideSize = 20;
	Rectangle<Color, Size> slide{};
	T number{};

	bool activeToMove = false;
	Vector2s lastFingerPosition{};
};
