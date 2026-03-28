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

	class PlayList
	{
	public:
		~PlayList()
		{
			if (songPath) ESP_LOGW(TAG, "song path = %p, not nullptr, %s", songPath, songPath);
		}

		constexpr auto getId() const { return id; }
		constexpr const char* getPath() const { return songPath; }
		constexpr const PlayList* getNext() const { return next; }
		constexpr const PlayList* getLast() const { return last; }
		constexpr const PlayList* getRandomNext() const { return randomNext; }
		constexpr const PlayList* getRandomLast() const { return randomLast; }
		constexpr PlayList* getNext() { return next; }
		constexpr PlayList* getLast() { return last; }
		constexpr PlayList* getRandomNext() { return randomNext; }
		constexpr PlayList* getRandomLast() { return randomLast; }

	private:
		constexpr static char TAG[] = "PlayList";
		friend class AudioServer;

		unsigned id = -1;

		const char* songPath{};

		PlayList* next{};
		PlayList* last{};

		PlayList* randomNext{};
		PlayList* randomLast{};
	};

	static PlayList* getPlayList();
	static PlayList* getPlayListNow();
	static bool isPlayListEnabled();
	static void enablePlayList();
	static void disablePlayList();
	static void addPlayList(const char* path, PlayList* insert = nullptr);
	static void removePlayList(PlayList* playList);
	static void changePlayList(PlayList* playList, const char* path);
	static void switchToNextPlayList();
	static void switchToLastPlayList();
	static void switchToPlayList(PlayList* playList);

	static PlayList* getPlayListRandom();
	static bool isRandomPlayEnabled();
	static void enableRandomPlay();
	static void disableRandomPlay();
	static void shufflePlayList();

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

	EXT_RAM_BSS_ATTR static PlayList* playListHead;
	EXT_RAM_BSS_ATTR static PlayList* playListRandomHead;
	EXT_RAM_BSS_ATTR static PlayList* playListNow;
	EXT_RAM_BSS_ATTR static bool randomPlay;

	static void playListRandomInsert(PlayList* newList);

	static void loaderMain(void*);
	static void decoderMain(void*);
};
