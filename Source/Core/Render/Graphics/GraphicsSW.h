#pragma once

#include <vector>

#include "IGraphics.h"
#include "Core/Math/Plane.h"

using namespace std;

class Object;

class GraphicsSW : public IGraphics
{
public:
	// Inherited via IGraphics
	HRESULT Init(const HWND& hWnd) override;
	void Render() override;
	void Release() override;

private:
	HWND mWnd = NULL;
	HDC mDC = NULL;
	HDC mMemDC = NULL;
	HBITMAP mRenderTarget = NULL;
	UINT mWindowWidth = 0;
	UINT mWindowHeight = 0;
	UINT* mPixelBuffer = nullptr;

	Plane mFrustomPlanes[6];

	Object* mCamera = nullptr;
	vector<Object*> mObjects;

	// Inherited via IGraphics
	virtual void AddObject(Object* object);

	// Inherited via IGraphics
	void SetCamera(Object* object) override;
};

