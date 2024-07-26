#pragma once

#include <mmsystem.h>

#include "Core/Types.h"

constexpr uint MAX_SAMPLE_COUNT = 50;
class GameTimer
{
public:
	GameTimer();
	virtual ~GameTimer() = default;

	void Tick(float lockFPS = 0.0f);
	uint32 GetFrameRate();
	float GetTimeElapsed();

private:
	bool mbHardwareHasPerformanceCounter;
	float mTimeScale	= 0.0f;
	float mTimeElapsed	= 0.0f;
	int64 mCurrentTime	= 0;
	int64 mLastTime		= 0;
	int64 mPerformanceFrequency = 0;

	float mFrameTime[MAX_SAMPLE_COUNT] = {};
	uint32 mSampleCount = 0;

	uint32 mCurrentFrameRate = 0;
	uint32 mFramePerSecond = 0;
	float mFPSTimeElapsed = 0.0f;
};