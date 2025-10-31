#include "utf8.hpp"

class Character
{
public:
	Unicode unicode{};
	unsigned char font[32]{};

	operator Unicode() const { return unicode; }
};

extern Character gbkCharacter[24066];
