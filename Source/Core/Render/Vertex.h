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

struct Mesh
{
	Vertex* Vertices = nullptr;
	uint VertexCount = 0;

	int* Indices	= nullptr;
	uint IndexCount = 0;

	Mesh(){}

	template <typename V, typename I>
	Mesh(V* vBuffer, uint vCount, I* iBuffer, uint iCount)
	{
		VertexCount = vCount;
		Vertices = new Vertex[vCount];
		for (uint i = 0; i < vCount; ++i)
		{
			Vertices[i] = { (float)vBuffer[i].Buffer()[0], (float)vBuffer[i].Buffer()[1], (float)vBuffer[i].Buffer()[2] };
		}

		IndexCount = iCount;
		Indices = new int[iCount];
		for (uint i = 0; i < iCount; ++i)
		{
			Indices[i] = iBuffer[i];
		}
	}

	~Mesh()
	{
		delete[] Vertices;
		Vertices = nullptr;
		VertexCount = 0;

		delete[] Indices;
		Indices = nullptr;
		IndexCount = 0;
	}
};