#include "audio.hpp"

#include "audio/mp3.hpp"
#include "audio/iis.hpp"
#include "audio/test.inl"

void AppAudio::init()
{
	audioTest("music/16b-2c-48000hz.mp3");
}
