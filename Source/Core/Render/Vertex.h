#pragma once
#include "Core/Math/Vector.h"
#include "Core/Types.h"

struct Vertex
{
	FVector Position;
	FVector Color = 1.0f;
	
	Vertex() {}
	Vertex(const FVector v, const FVector c) : Position(v), Color(c) {}
	Vertex(float x, float y, float z) : Position(x,y,z){}
	Vertex(float num) : Position(num) {}
};