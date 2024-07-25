#pragma once
#include <Windows.h>

class Object;

class IGraphics
{
public:
	virtual HRESULT Init(const HWND& hWnd) = 0;
	virtual void Render() = 0;
	virtual void Release() = 0;

	virtual void ResizeWindow(UINT width, UINT height) = 0;
	virtual void AddObject(Object* object) = 0;
	virtual void SetCamera(Object* object) = 0;
};