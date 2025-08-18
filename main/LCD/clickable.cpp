#include "clickable.hpp"

void Clickable::finger(Finger finger)
{
	using State = Finger::State;
	if (isClicked(finger.position))
	{
		switch (finger.state)
		{
		case State::Press: pressCallback(finger, clickCallbackParam); break;
		case State::Contact: holdCallback(finger, clickCallbackParam); break;
		case State::Realease: releaseCallback(finger, clickCallbackParam); break;
		case State::None: break;
		}
	}
}
