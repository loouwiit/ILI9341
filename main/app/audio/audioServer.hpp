#include "audio/mp3.hpp"
#include "audio/iis.hpp"

class AudioServer
{
public:
	static bool isPaused();
	static void turnOff();
	static void turnOn();

	static bool isInited();
	static void init();
	static void deinit();

	static const char* getPlayingPath();
	static bool isPlaying();
	static void play(const char* path);

private:
	constexpr static size_t FrameBufferLength = 8192;

	EXT_RAM_BSS_ATTR static char path[256];
	EXT_RAM_BSS_ATTR static MP3* mp3Loader;
	EXT_RAM_BSS_ATTR static uint8_t* frameBuffer;

	EXT_RAM_BSS_ATTR static GPIO SD;
	EXT_RAM_BSS_ATTR static IIS iis;

	EXT_RAM_BSS_ATTR static bool audioPause;
	EXT_RAM_BSS_ATTR static TaskHandle_t audioServerHandle;
	EXT_RAM_BSS_ATTR static StackType_t* audioServerStack;
	EXT_RAM_BSS_ATTR static StaticTask_t* audioServerTask; // must in internal ram

	static void serverMain(void*);
};
