#include "pch.h"
#include "Plane.h"

PlaneRelation CalculatePointPlaneRelation(const Plane& plane, const FVector& position)
{
	FVector relativePos = position - plane.Pos;
	float result = plane.Normal.Dot(relativePos);

	if (result > 0)
		return PlaneRelation::PP_Front;
	else if (result == 0)
		return PlaneRelation::PP_On;
	else
		return PlaneRelation::PP_Back;
}

