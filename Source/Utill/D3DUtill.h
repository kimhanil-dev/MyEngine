#pragma once

#include <Windows.h>
#include <sstream>
#include <format>

#if defined(DEBUG) | (_DEBUG)
	#ifndef HR
	#define HR(x) \
	{ HRESULT hr = (x); \
		if(FAILED(hr)) \
		{\
			LPWSTR output; \
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&output, 0, NULL); \
			std::wostringstream outs;\
			outs << std::format(L"File: {}\nLine: {}\nFunc: {}\nError Msg: {}", TEXT(__FILE__), (DWORD)__LINE__, L#x, output);\
			MessageBox(NULL, outs.str().c_str(), TEXT("ERROR"), MB_OK);\
		}\
	}
	#endif
#else
	#ifndef HR
	#define HR(x) (x)
	#endif
#endif