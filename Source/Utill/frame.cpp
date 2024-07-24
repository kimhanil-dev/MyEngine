#include "frame.h"

#include <Windows.h>

int GetFPS()
{
	static ULONGLONG prevTime = GetTickCount64();
	static ULONGLONG curTime = 0;
	static float accumTime = 0;
	static int counter = 0;
	static int fps = 0;

	curTime = GetTickCount64();
	float deltaTime = (curTime - prevTime) * 0.001f;
	prevTime = curTime;

	++counter;
	accumTime += deltaTime;
	if (accumTime >= 1.0f)
	{
		accumTime -= 1.0f;
		fps = counter;
		counter = 0;
	}

	return fps;
}