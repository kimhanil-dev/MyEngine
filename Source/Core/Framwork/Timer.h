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
private:
	double mSecondsPerCount;
	double mDeltaTime;
	int64 mBaseTime;
	int64 mPausedTime;
	int64 mStopTime;
	int64 mPrevTime;
	int64 mCurrTime;
	bool mbStopped;
};