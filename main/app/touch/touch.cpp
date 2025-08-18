#include "touch.hpp"

void AppTouch::init()
{
	App::init();

	line1Clear[0] = &line1XClear;
	line1Clear[1] = &line1YClear;
	line1[0] = &line1X;
	line1[1] = &line1Y;

	line2Clear[0] = &line2XClear;
	line2Clear[1] = &line2YClear;
	line2[0] = &line2X;
	line2[1] = &line2Y;
}

void AppTouch::draw()
{
	lcd.draw(number);
	lcd.draw(state[0]);
	lcd.draw(state[1]);
	lcd.draw(line1Clear);
	lcd.draw(line2Clear);
	if (state[0].number != 0x04)
		lcd.draw(line1);
	if (state[1].number != 0x04)
		lcd.draw(line2);

	line1XClear.end.y = (line1XClear.start.y = line1X.start.y) + 1;
	line1YClear.end.x = (line1YClear.start.x = line1Y.start.x) + 1;
	line2XClear.end.y = (line2XClear.start.y = line2X.start.y) + 1;
	line2YClear.end.x = (line2YClear.start.x = line2Y.start.x) + 1;
}

void AppTouch::touchUpdate()
{
	number.number++;

	state[0].number = (unsigned)touch[0].state;
	state[1].number = (unsigned)touch[1].state;
	line1X.end.y = (line1X.start.y = touch[0].position.y) + 1;
	line1Y.end.x = (line1Y.start.x = touch[0].position.x) + 1;

	line2X.end.y = (line2X.start.y = touch[1].position.y) + 1;
	line2Y.end.x = (line2Y.start.x = touch[1].position.x) + 1;
}
