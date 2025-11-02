#pragma once

#include "font.hpp"

class AutoLnaguage
{
public:
	const char* english = "";
	const char* chinese = "";

	constexpr AutoLnaguage() = default;
	constexpr AutoLnaguage(const char* english) : english{ english }, chinese{ english } {}
	constexpr AutoLnaguage(const char* english, const char* chinese) : english{ english }, chinese{ chinese } {}
	constexpr AutoLnaguage(AutoLnaguage&&) = default;
	constexpr AutoLnaguage(AutoLnaguage&) = default;
	constexpr AutoLnaguage& operator=(AutoLnaguage&&) = default;
	constexpr AutoLnaguage& operator=(AutoLnaguage&) = default;

	operator const char* () const
	{
		return fontChinese != fontNone ? chinese : english;
	}
};
