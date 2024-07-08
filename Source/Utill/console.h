#pragma once

// CMD에 문자열을 출력하는 기능을 제공함
// 편의를 위하여 간단하게 작성된 코드
#include "Window/framework.h"

#ifdef _DEBUG
	#define Print(format, ...) console::printf(console::Color::white, format, __VA_ARGS__)
	#define PrintLog(type,color, format, ...) console::printf(color, "["#type"]""[file:%s][func:%s][line:%d] : "#format"\n", __FILE__, __FUNCTION__, __LINE__,__VA_ARGS__)
	#define PrintWarning(format, ...) PrintLog(Warning, console::Color::yellow, format, __VA_ARGS__)
	#define PrintError(format, ...) PrintLog(Error, console::Color::red, format, __VA_ARGS__)
	#define PrintGood(format, ...) PrintLog(Good, console::Color::green, format, __VA_ARGS__)
#elif
	#define print(str, color)
#endif

namespace console
{
	enum Color
	{
		start	= 1,

		blue	= 9,
		green	= 10,
		red		= 12,
		yellow	= 14,
		white	= 15,

		end		= 254
	};

	// https://stackoverflow.com/questions/4053837/colorizing-text-in-the-console-with-c
	template <typename ...Types>
	void printf(const Color color, const char* format, Types... args)
	{
		static bool isInited = false;
		static HANDLE hConsole = NULL;
		if (!isInited)
		{
			AllocConsole();
			freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);

			hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		}

		SetConsoleTextAttribute(hConsole, color);
		printf_s(format, args...);
		SetConsoleTextAttribute(hConsole, Color::white);
	}
}
