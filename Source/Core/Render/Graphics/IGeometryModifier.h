#pragma once
#include "Core/Math/Matrix.h"

class IGeometryModifier
{
public:
	virtual void SetFloat(const char* name, const float& value) = 0;
	virtual void SetTransform(const FMatrix4x4& worldTransMat) = 0;
};