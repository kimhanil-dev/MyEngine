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
	bool Init(const HWND& hWnd) override;
	void Render() override;
	void Release() override;

private:
	HWND mWnd = NULL;
	HDC mDC = NULL;
	HDC mMemDC = NULL;
	HBITMAP mRenderTarget = NULL;
	uint mWindowWidth = 0;
	uint mWindowHeight = 0;
	uint* mPixelBuffer = nullptr;

	Plane mFrustomPlanes[6];

	Object* mCamera = nullptr;
	vector<Object*> mObjects;

	// Inherited via IGraphics
	virtual void AddObject(Object* object);

	// Inherited via IGraphics
	void SetCamera(Object* object) override;

	// IGraphics을(를) 통해 상속됨
	void ResizeWindow(uint width, uint height) override;

	// Inherited via IGraphics
	bool bIsInited = false;
	bool IsInited() override;

	// IGraphics을(를) 통해 상속됨
	IGeometryModifier* BindMesh(Mesh* mesh) override;
};

