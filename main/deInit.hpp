#pragma once

template<class T>
class Deinit
{
public:
	Deinit(T& t) : t{ t } {};
	Deinit(T&& t) : t{ t } {};
	~Deinit() { t(); }
private:
	T t;
};
