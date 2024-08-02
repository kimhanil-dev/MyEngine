#pragma once
#include <Windows.h>
#include <memory>

#include "Core/Math/Matrix.h"
#include "Core/Types.h"

class Object;
class IGeometryModifier;
struct Mesh;

using namespace std;

class IGraphics
{
public:
	virtual bool Init(const HWND& hWnd) = 0;
	virtual void Render() = 0;
	virtual void Release() = 0;
	virtual weak_ptr<IGeometryModifier> BindMesh(Mesh* mesh) = 0;
	
	virtual void ResizeWindow(uint width, uint height) = 0;
	virtual void AddObject(Object* object) = 0;
	virtual void SetView(FMatrix4x4 viewMatrix) = 0;

	virtual bool IsInited() = 0;
};