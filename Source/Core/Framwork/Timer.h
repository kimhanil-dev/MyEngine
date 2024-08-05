#pragma once

#include <mmsystem.h>

#include "Core/Types.h"

class GameTimer
{
public:
	GameTimer();
	float TotalTime() const;
	float GameTime() const; // in seconds
	float DeltaTime() const; // in seconds
	void Reset();	// call before message loop.
	void Stop();
	void Start();	// call when unpaused.
	void Tick();	// call every frame.

	// 성능 측정을 위한 유틸함수 입니다.
	// StartPerformance() 호출
	// 작업...
	// EndPerformance()
	// 위의 형태로 사용합니다.
	void StartPerformance();
	float EndPerformance();
private:
	double mSecondsPerCount;
	double mDeltaTime;
	int64 mBaseTime;
	int64 mPausedTime;
	int64 mStopTime;
	int64 mPrevTime;
	int64 mCurrTime;
	bool mbStopped;

	int64 mPerformanceStart;
};