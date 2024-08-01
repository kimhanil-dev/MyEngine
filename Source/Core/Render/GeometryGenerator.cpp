#include "pch.h"
#include "GeometryGenerator.h"

#include "Core/Math/MathDefine.h"
#include <math.h>

void GeometryGenerator::CreateGrid(float width, float depth, UINT m, UINT n, Mesh& outMesh)
{
	UINT vertexCount = m * n;
	UINT faceCount = (m - 1) * (n - 1) * 2;

	float halfWidth = 0.5f * width;
	float halfDepth = 0.5f * depth;

	float dx = width / (n - 1);
	float dz = depth / (m - 1);
	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	// because m x n verices Grid's cell count (m - 1) * (n - 1), add +1
	outMesh.Vertices.resize(vertexCount);
	for (UINT i = 0; i < m; ++i)
	{
		float z = halfDepth - i * dz;
		for (UINT k = 0; k < n; ++k)
		{
			float x = -halfWidth + k * dx;
			UINT index = i * n + k;
			outMesh.Vertices[index].Position = { x, 0.0f, z };
			outMesh.Vertices[index].Color = { 1.0f,1.0f,1.0f };
			outMesh.Vertices[index].U = k * du;
			outMesh.Vertices[index].V = i * dv;
		}
	}

	outMesh.Indices.resize(faceCount * 3);
	UINT index = 0;
	for (UINT i = 0; i < m - 1; ++i)
	{
		for (UINT k = 0; k < n - 1; ++k)
		{
			outMesh.Indices[index] = i * n + k;
			outMesh.Indices[index + 1] = i * n + k + 1;
			outMesh.Indices[index + 2] = (i + 1) * n + k;

			outMesh.Indices[index + 3] = (i + 1) * n + k;
			outMesh.Indices[index + 4] = i * n + k + 1;
			outMesh.Indices[index + 5] = (i + 1) * n + k + 1;

			index += 6;
		}
	}
}

void GeometryGenerator::CreateCylinder(float topRadius, float bottomRadius, float height, Mesh& outMesh)
{
	// top


}

void GeometryGenerator::CreateCircle(float radius, UINT segment, Mesh& outMesh)
{
	assert(segment >= 3);

	UINT vertexCount = segment + 1;

	float dRadian = PI / segment;

	outMesh.Vertices.resize(vertexCount);

	outMesh.Vertices[0] = Vertex(FVector(0.0f), FVector(0.0f));
	for (UINT i = 1; i < segment; ++i)
	{
		float radian = dRadian * i;
		outMesh.Vertices[i] = Vertex(FVector{ cosf(radian), 0.0f, sinf(radian) }, FVector{ 1.0f,1.0f,1.0f });
	}

}