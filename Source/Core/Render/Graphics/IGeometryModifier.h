#pragma once
#include <functional>
#include "Core/Math/Matrix.h"


using namespace std;

struct D3D11_MAPPED_SUBRESOURCE;

class IGeometryModifier
{
public:
	virtual void SetFloat(const char* name, const float& value) = 0;
	virtual void SetFloat3(const char* name, const float* value) = 0;
	virtual void SetMatrix(const char* name, const FMatrix4x4& value) = 0;
};


class IGeometryDynamicModifier : public IGeometryModifier
{
public:
	virtual void BindResourceMap(function<void(D3D11_MAPPED_SUBRESOURCE data)> mapAction) = 0;
};