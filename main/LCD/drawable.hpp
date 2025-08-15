#pragma once

#include "noneCopyable.hpp"
#include "vector.hpp"
#include "esp_log.h"

template <class Child>
class Drawable
{
public:
	Vector2us drawTo(auto& target) const
	{
		return (static_cast<Child>(*this)).drawTo(std::ref(target));
	}
};

template <class TargetChild>
class DrawTarget : public NoneCopyable
{
public:
	template <class T>
	Vector2us draw(T&& element)
		requires
	(
		requires(T element) { element.drawTo(static_cast<TargetChild>(*this)); }
		)
	{
		// 若element能draw直接draw
		return element.drawTo(static_cast<TargetChild>(*this));
	}

	template <class T>
	Vector2us draw(T&& element)
		requires
	(
		!requires(T element) { element.drawTo(static_cast<TargetChild>(*this)); }&&
		requires(T element) { static_cast<TargetChild>(*this).draw(std::forward(element)); }
		)
	{
		// 若element没draw试试自己的子类
		return static_cast<TargetChild>(*this).draw(std::forward(element));
	}

	template <class T>
	Vector2us draw(T&& element)
		requires
	(
		!requires(T element) { element.drawTo(static_cast<TargetChild>(*this)); } &&
		!requires(T element) { static_cast<TargetChild>(*this).draw(std::forward(element)); }
		)
	{
		// 要么 element.drarTo(*this) 能行
		// 要么 (子类).draw(element) 能行
		static_assert(false, "DrawTarget的draw匹配失败");
		return { 0,0 };
	}
};
