#pragma once
#include "Core/Math/Matrix.h"

class IGeometryModifier
{
public:
	virtual void SetTransform(const FMatrix4x4& worldTransMat) = 0;
};