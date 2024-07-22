#pragma once

#include <Windows.h>

enum class Renderer : unsigned int
{
	DriectX,
	Software,
	// add renderer here
	
	// ...
	Count,
};

class Object;

class IGraphics
{
public:
	virtual HRESULT Init(const HWND& hWnd) = 0;
	virtual void Render() = 0;
	virtual void Release() = 0;

	virtual void AddObject(Object* object) = 0;
	virtual void SetCamera(Object* object) = 0;
};

IGraphics* GetRenderer(Renderer renderer);