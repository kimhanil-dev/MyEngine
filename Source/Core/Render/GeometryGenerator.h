#pragma once
#include "Core/Render/Mesh.h"

class GeometryGenerator
{
public:

	void CreateGrid(float width, float depth, UINT m, UINT n, Mesh& outMesh);

	void CreateCylinder(const float topRadius, const float bottomRadius, const float height, 
		const UINT circleSeg, const UINT cylinderSeg, Mesh& outMesh);				
	
	void CreateGeosphere(const XMFLOAT4& color,float radius, UINT numSubdivisions, Mesh& outMesh);

	void CreateCircle(Mesh& outMesh, const float radius, const UINT segment, bool isCenterAtStart = false);

	void Subdivide(Mesh& outMesh);
};

