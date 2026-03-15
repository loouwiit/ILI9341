#include "audio/decoder.hpp"
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

	static void setGain(int8_t gain);
	static int8_t getGain();

private:
	constexpr static size_t FrameBufferLength = 8192;
	constexpr static size_t DecoderBufferLength = 4096;
	constexpr static size_t DecoderBufferThresholdDefault = 256;
	constexpr static size_t DecoderBufferThresholdIncrease = 128;
	constexpr static size_t DecoderBufferThresholdMax = 2048;

	EXT_RAM_BSS_ATTR static bool autoDeinit;

	EXT_RAM_BSS_ATTR static char path[256];
	EXT_RAM_BSS_ATTR static Decoder* decoder;
	EXT_RAM_BSS_ATTR static uint8_t* decoderBuffer;
	EXT_RAM_BSS_ATTR static size_t decoderBufferThreshold;
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
