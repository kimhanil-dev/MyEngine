#pragma once
#include <functional>
#include <DirectXMath.h>

using namespace std;
using namespace DirectX;

struct D3D11_MAPPED_SUBRESOURCE;

class IGeometryModifier
{
public:
	virtual void SetRaw(const char* name, const void* data, const uint32_t byteSize) = 0;
	virtual void SetFloat(const char* name, const float& value) = 0;
	virtual void SetFloat3(const char* name, const float* value) = 0;
	virtual void SetMatrix(const char* name, const XMFLOAT4X4& value) = 0;
};


class IGeometryDynamicModifier : public IGeometryModifier
{
public:
	virtual void BindResourceMap(function<void(D3D11_MAPPED_SUBRESOURCE data)> mapAction) = 0;
};