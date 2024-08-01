#pragma once
#include "Core/Math/Vector.h"
#include "Core/Types.h"

struct Vertex
{
	FVector Position;
	FVector Color;
	float U = 0.0f, V = 0.0f;
	
	Vertex(){}
	Vertex(const FVector v, const FVector c, float tu = 0.0f, float tv = 0.0f) : Position(v), Color(c), U(tu), V(tv) {}
	Vertex(float x, float y, float z) : Position(x,y,z){}
	Vertex(float num) : Position(num) {}
};