#pragma once

#include "app/app.hpp"
#include "LCD/autoLanguage.hpp"

class AppStrip final : public App
{
public:
	constexpr static auto GpioNum = GPIO_NUM_1;
	constexpr static uint32_t LedCount = 15;

	AppStrip(LCD& lcd, FT6X36& touch, Callback_t changeAppCallback, Callback_t newAppCallback) : App(lcd, touch, changeAppCallback, newAppCallback) {}

	virtual void init() override final;
	// virtual void focusIn() override final;
	virtual void deinit() override final;

	virtual void draw() override final;
	virtual void touchUpdate() override final;
	virtual void back() override final;

private:
	constexpr static unsigned char TitleSize = 3;
	constexpr static unsigned char TextSize = 2;
	constexpr static short GapSize = 8;
	constexpr static short ContentXOffset = 20;
	constexpr static LCD::Color BackgroundColor = { 0x20,0x20,0x20 };
	constexpr static short BoardSize = 3;
	constexpr static TickType_t LastTimeStep = 10;

	constexpr static unsigned char ContensSize = 8;
	LCD::Layar<LayarClassicSize::Middle> contents{ ContensSize };
	LCD::Text title{ {LCD::ScreenSize.x / 2, 0}, AutoLnaguage{"strip", "灯带"}, TitleSize };
	LCD::Text stripText{ {ContentXOffset, 16 * TitleSize + GapSize + (16 * TextSize + GapSize) * 0}, AutoLnaguage{"strip:error", "灯带:错误"}, TextSize, LCD::Color::White, BackgroundColor };

	LCD::Text loadText{ {ContentXOffset, 16 * TitleSize + GapSize + (16 * TextSize + GapSize) * 1}, AutoLnaguage{"load file", "加载文件"}, TextSize, LCD::Color::White, BackgroundColor };
	LCD::Text saveText{ {ContentXOffset, 16 * TitleSize + GapSize + (16 * TextSize + GapSize) * 2}, AutoLnaguage{"save file", "保存文件"}, TextSize, LCD::Color::White, BackgroundColor };

	LCD::Layar<LayarClassicSize::Small> stepLayar{ {ContentXOffset, 16 * TitleSize + GapSize + (16 * TextSize + GapSize) * 3},Vector2s{(short)LCD::ScreenSize.x, 16 * TextSize},5 };
	char stepTextBuffer[11];
	LCD::Text stepText{ {0,0}, AutoLnaguage{ "step:nnn", "步骤:nnn" }, TextSize, LCD::Color::White, BackgroundColor };
	LCD::Text stepLeft{ {(short)(stepText.position.x + stepText.computeSize().x + GapSize),0}, "<", TextSize, LCD::Color::White, BackgroundColor, &fontsFullWidth };
	LCD::Text stepRight{ {(short)(stepLeft.position.x + stepLeft.computeSize().x + GapSize),0}, ">", TextSize, LCD::Color::White, BackgroundColor, &fontsFullWidth };
	LCD::Text stepAdd{ {(short)(stepRight.position.x + stepRight.computeSize().x + GapSize),0}, "+", TextSize, LCD::Color::White, BackgroundColor, &fontsFullWidth };
	LCD::Text stepRemove{ {(short)(stepAdd.position.x + stepAdd.computeSize().x + GapSize),0}, "-", TextSize, LCD::Color::White, BackgroundColor, &fontsFullWidth };

	LCD::Layar<LayarClassicSize::Small> lastTimeLayar{ {ContentXOffset, 16 * TitleSize + GapSize + (16 * TextSize + GapSize) * 4},Vector2s{(short)LCD::ScreenSize.x, 16 * TextSize},3 };
	char lastTimeTextBuffer[14];
	LCD::Text lastTimeText{ {}, AutoLnaguage{ "time:xxxxms", "时长:xxxxms" }, TextSize, LCD::Color::White, BackgroundColor };
	LCD::Text lastTimeAdd{ {stepAdd.position.x,0}, "+", TextSize, LCD::Color::White, BackgroundColor, &fontsFullWidth };
	LCD::Text lastTimeSub{ {stepRemove.position.x,0}, "-", TextSize, LCD::Color::White, BackgroundColor, &fontsFullWidth };

	constexpr static short SlideSize = 16 * TextSize;
	LCD::Bar<int> lastTimeBar{ Vector2s{ContentXOffset, 16 * TitleSize + GapSize + (16 * TextSize + GapSize) * 5} + Vector2s{SlideSize, SlideSize} / 2, 250, SlideSize - 10, SlideSize, BackgroundColor, LCD::Color::White };

	LCD::Layar<LayarClassicSize::Huge> ledLayar{ Vector2s{0,16 * TitleSize + GapSize + (16 * TextSize + GapSize) * 6},Vector2s{(short)LCD::ScreenSize.x, 16 * TextSize}, LedCount * 2 };
	LCD::Rectangle ledBoards[LedCount]{};
	LCD::Rectangle leds[LedCount]{};
	AppStrip* ledParams[LedCount]{};

	constexpr static char ConfigFilePath[] = "/root/mem/stripAutoSave.cfg";

	void updateState();

	App* appColorInput = nullptr;

	constexpr static float moveThreshold2 = 100.0f;

	bool fingerActive[2] = { false, false };
	bool fingerMoveLeds[2]{};
	bool fingerClickTime[2]{};
	Vector2s lastFingerPosition[2]{};
	Vector2s fingerMoveTotol[2]{};

	void click(Finger finger);
	void releaseDetect();
};
