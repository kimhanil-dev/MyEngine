#pragma once
#include <Windows.h>
#include <memory>
#include <DirectXMath.h>

#include "Core/Types.h"	

#include "Lights/PointLight.h"

class IObject;
class IGeometryModifier;
class IGeometryDynamicModifier;
struct Mesh;

using namespace std;
using namespace DirectX;

class IGraphics
{
public:
	virtual bool Init(const HWND& hWnd) = 0;
	virtual void Render() = 0;
	virtual void Release() = 0;

	virtual weak_ptr<IGeometryModifier> BindMesh(Mesh* mesh) = 0;
	virtual weak_ptr<IGeometryDynamicModifier> BindMeshDynamic(Mesh* mesh) = 0;

	virtual void ResizeWindow(uint width, uint height) = 0;

	virtual void BindCameraObject(const IObject* cameraObject) = 0;
	virtual void UnBindCameraObject(const IObject* cameraObject) = 0;

	virtual void BindPointLight(const IObject* object, PointLight& desc) = 0;
	virtual void UnBindPointLight(const IObject* lightObject) = 0;

	virtual bool IsInited() = 0;
};