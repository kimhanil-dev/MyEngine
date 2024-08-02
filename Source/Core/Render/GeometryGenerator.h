#pragma once
#include "Core/Render/Mesh.h"

class GeometryGenerator
{
public:

	void CreateGrid(float width, float depth, UINT m, UINT n, Mesh& outMesh);
	void CreateCylinder(const float topRadius, const float bottomRadius, const float height, 
		const UINT circleSeg, const UINT cylinderSeg, Mesh& outMesh);				
	void CreateCylinderV2(const float topRadius, const float bottomRadius, const float height,
		const UINT circleSeg, const UINT cylinderSeg, Mesh& outMesh);
	// bIsCenterAtStart : 원의 중심에 대한 Vertex를 Vertex Buffer의 첫번째에 둘 것인지, 마지막에 둘 것인지에 대한 변수
	// CreateCylinder에서 
	void CreateCircle(Mesh& outMesh, const float radius, const UINT segment, bool bIsCenterAtStart = false);
};

