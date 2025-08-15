#pragma once

class NoneCopyable
{
public:
	NoneCopyable() = default;

	NoneCopyable(NoneCopyable&&) = default;
	NoneCopyable& operator=(NoneCopyable&&) = delete;

	NoneCopyable(NoneCopyable&) = delete;
	NoneCopyable& operator=(NoneCopyable&) = delete;
};
