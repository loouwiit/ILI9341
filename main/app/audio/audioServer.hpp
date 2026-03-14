#include "audio/mp3.hpp"
#include "audio/alc.hpp"
#include "audio/iis.hpp"
#include "task.hpp"

class AudioServer
{
public:
	static void pause();
	static void resume();
	static bool isPaused();

	static bool isInited();
	static void init();
	static void deinit();
	static void setAutoDeinit(bool status);

	static const char* getFilePath();
	static bool isOpened();
	static void openFile(const char* path);
	static void close();

	static void setGain(uint8_t gain);
	static uint8_t getGain();

private:
	constexpr static size_t FrameBufferLength = 8192;
	constexpr static size_t MP3BufferLength = 4096;
	constexpr static size_t MP3BufferSThrehood = 250;

	EXT_RAM_BSS_ATTR static bool autoDeinit;

	EXT_RAM_BSS_ATTR static char path[256];
	EXT_RAM_BSS_ATTR static MP3* mp3Loader;
	EXT_RAM_BSS_ATTR static uint8_t* mp3Buffer;
	EXT_RAM_BSS_ATTR static ALC* alc;
	EXT_RAM_BSS_ATTR static uint8_t* alcBuffer;

	EXT_RAM_BSS_ATTR static GPIO SD;
	EXT_RAM_BSS_ATTR static IIS iis;

	EXT_RAM_BSS_ATTR static bool audioPause;
	EXT_RAM_BSS_ATTR static bool serverRunning;

	EXT_RAM_BSS_ATTR static Thread decoderThread;
	EXT_RAM_BSS_ATTR static Thread loaderThread;
	EXT_RAM_BSS_ATTR static bool decoderPause;
	EXT_RAM_BSS_ATTR static bool loaderPause;

	static void loaderMain(void*);
	static void decoderMain(void*);
};
