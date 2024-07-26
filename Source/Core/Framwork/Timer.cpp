#include "pch.h"
#include "Timer.h"

GameTimer::GameTimer()
{
	// 성능 카운터의 초당 개수(Frequency)를 받아오는 함수
	if (QueryPerformanceFrequency((LARGE_INTEGER*)&mPerformanceFrequency))
	{
		mbHardwareHasPerformanceCounter = true;
		// 성능 카운터의 현재 값을 반환함
		QueryPerformanceCounter((LARGE_INTEGER*)&mLastTime); // 
		mTimeScale = 1.0f / mPerformanceFrequency;
	}
	else
	{
		mbHardwareHasPerformanceCounter = false;
		mLastTime = ::timeGetTime();
		mTimeScale = 0.001f;
	}
}

void GameTimer::Tick(float lockFPS)
{
	if (mbHardwareHasPerformanceCounter)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)mCurrentTime);
	}
	else
	{
		mCurrentTime = ::timeGetTime();
	}

	float timeElapsed = (mCurrentTime - mLastTime) * mTimeScale;
	if (lockFPS > 0.0f)
	{
		while (timeElapsed < (1.0f / lockFPS))
		{
			if (mbHardwareHasPerformanceCounter)
			{
				QueryPerformanceCounter((LARGE_INTEGER*)mCurrentTime);
			}
			else
			{
				mCurrentTime = ::timeGetTime();
			}
			
			timeElapsed = (mCurrentTime - mLastTime) * mTimeScale;
		}
	}

	mLastTime = mCurrentTime;
	if (fabsf(timeElapsed - mTimeElapsed) < 1.0f)
	{
		// 현재 프레임 처리 시간 저장
		memmove(&mFrameTime[1], mFrameTime, (MAX_SAMPLE_COUNT - 1) * sizeof(float));
		mFrameTime[0] = timeElapsed;
		if (mSampleCount < MAX_SAMPLE_COUNT) ++mSampleCount;
	}

	++mFramePerSecond;
	// 현재 프레임 처리 시간을 누적하여 저장
	mFPSTimeElapsed += timeElapsed;
	if (mFPSTimeElapsed > 1.0f)
	{
		mCurrentFrameRate = mFramePerSecond;
		mFramePerSecond = 0;
		mFPSTimeElapsed -= 1.0f;
	}

	mTimeElapsed = 0.0f;
	for (uint32 i = 0; i < mSampleCount; ++i)
		mTimeElapsed += mFrameTime[i];

	if (mSampleCount > 0)
		mTimeElapsed /= mSampleCount;
}

uint32 GameTimer::GetFrameRate()
{
	return uint32();
}

float GameTimer::GetTimeElapsed()
{
	return 0.0f;
}
