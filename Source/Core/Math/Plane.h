#pragma once
#include "Core/Math/Vector.h"

enum class PlaneRelation
{
	PP_Front,
	PP_On,
	PP_Back,
};

struct Plane
{
	FVector Normal;
	FVector	Pos;
};

PlaneRelation CalculatePointPlaneRelation(const Plane& plane, const FVector& position);