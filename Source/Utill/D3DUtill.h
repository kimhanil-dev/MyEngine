#pragma once

#include <Windows.h>
#include <sstream>
#include <format>

#if defined(DEBUG) | (_DEBUG)
	#ifndef IF_FAILED_RET
	#define IF_FAILED_RET(x)	\
	{						\
		HRESULT hr = (x);	\
		if (FAILED(hr))		\
			return hr;		\
	}
	#endif

	#ifndef HR
	#define HR(x) \
	{ HRESULT hr = (x); \
		if(FAILED(hr)) \
		{\
			LPWSTR output; \
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&output, 0, NULL); \
			std::wostringstream outs;\
			outs << std::format(L"File: {}\nLine: {}\nError Code: 0x{:X}\nCalling: {}\nError Msg: {}", TEXT(__FILE__), (DWORD)__LINE__, (DWORD)hr, L#x, output);\
			MessageBox(NULL, outs.str().c_str(), TEXT("ERROR"), MB_OK | MB_ICONERROR);\
			LocalFree(output);\
		}\
	}
	#endif
#else
	#ifndef HR
	#define HR(x) (x)
	#endif
	#ifndef IF_FAILED_RET
	#define IF_FAILED_RET(x) (x)
	#endif
#endif

namespace Colors
{
	XMGLOBALCONST XMVECTORF32 White = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Green = { 0.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Cyan = { 0.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };
}