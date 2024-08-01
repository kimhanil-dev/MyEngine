#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <vector>

#include "Core/Types.h"
#include "Vertex.h"

struct DXMesh
{
	Microsoft::WRL::ComPtr<ID3D11Buffer> mVB;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mIB;
};

using namespace std;

struct Mesh
{
	vector<Vertex> Vertices;
	vector<UINT> Indices;

	Mesh() {}
	Mesh(const vector<Vertex>& vertices, const vector<UINT>& indices)
		: Vertices(vertices)
		, Indices(indices)
	{}

	template <typename V, typename I>
	Mesh(V* vBuffer, uint vCount, I* iBuffer, uint iCount)
	{
		Vertices.resize(vCount);
		for (uint i = 0; i < vCount; ++i)
		{
			Vertices[i] = { (float)vBuffer[i].Buffer()[0], (float)vBuffer[i].Buffer()[1], (float)vBuffer[i].Buffer()[2] };
		}

		Indices.resize(iCount);
		memcpy(Indices.data(), iBuffer, sizeof(UINT) * iCount);
	}
};