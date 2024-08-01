#pragma once
#include "Core/Render/Mesh.h"

class GeometryGenerator
{
public:

	void CreateGrid(float width, float depth, UINT m, UINT n, Mesh& outMesh);
	void CreateCylinder(float topRadius, float bottomRadius, float height, Mesh& outMesh);
	void CreateCircle(float radius, UINT segment, Mesh& outMesh);
};

