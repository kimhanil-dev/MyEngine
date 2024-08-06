#include "pch.h"
#include "Vertex.h"

Vertex Vertex::operator+(const Vertex& v)
{
	Vertex result;
	result.Position + v.Position;
	result.Color + v.Color;
	
	return result;
}

Vertex Vertex::operator-(const Vertex& v)
{
	Vertex result;
	result.Position - v.Position;
	result.Color - v.Color;

	return result;
}
