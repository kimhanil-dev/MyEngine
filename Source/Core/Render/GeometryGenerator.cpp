#include "pch.h"
#include "GeometryGenerator.h"

#include "Core/Math/MathDefine.h"
#include <math.h>
#include <algorithm>

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

void GeometryGenerator::CreateCylinder(const float topRadius, const float bottomRadius, const float height,
	const UINT circleSeg, const UINT cylinderSeg, Mesh& outMesh)
{
	Mesh topMesh;
	CreateCircle(topMesh, topRadius, circleSeg);
	Mesh bottomMesh;
	CreateCircle(bottomMesh, bottomRadius, circleSeg);
	Mesh cylinderMesh;

	//// merge vertices
	//merge(topMesh.Vertices.begin(), topMesh.Vertices.end(),
	//	bottomMesh.Vertices.begin(), bottomMesh.Vertices.end(), 
	//	cylinderMesh.Vertices.begin());

	UINT cySeg = (cylinderSeg - 2); // cylinder의 top과 bottom의 vertex는 topMesh, bottomMesh와 중복이므로 감소 시킵니다.
	UINT faceCount = cylinderSeg * circleSeg * 2;// index는 cylinder의 top과 bottom을 topMesh와 bottomMesh로 연결해야 하므로 감소시키지 않습니다.

	// deltas
	float dY = height / (cylinderSeg - 1);	// cylinder seg 
	float dCySeg = 1.0f / (cylinderSeg - 1);

#pragma region column first creation
	{
		size_t cyVtxCount = cySeg * circleSeg;
		size_t tcVtxCount = topMesh.Vertices.size();
		size_t bcVtxCount = bottomMesh.Vertices.size();

		//***  merge된 vertex vector와 index vector의 순서는 
		// cylinder, 
		// bottom circle,
		// top circle  순 입니다.

		// merge를 위해 bottom의 index를 변형하고, topMesh vertices를 height만큼 높입니다.
		UINT bcIndexBias = cyVtxCount;
		UINT tcIndexBias = bcVtxCount + cyVtxCount;
		std::ranges::for_each(topMesh.Vertices, [height](Vertex& vertex) { vertex.Position.Y += height; }); // add height
		std::ranges::for_each(topMesh.Indices, [tcIndexBias](UINT& index) { index += tcIndexBias; }); // add index bias
		std::ranges::for_each(bottomMesh.Indices, [bcIndexBias](UINT& index) { index += bcIndexBias; }); // add index bias

		// concat top, cylinder, bottom vertices
		cylinderMesh.Vertices.resize(cyVtxCount);
		cylinderMesh.Vertices.insert(cylinderMesh.Vertices.end(), bottomMesh.Vertices.begin(), bottomMesh.Vertices.end());
		cylinderMesh.Vertices.insert(cylinderMesh.Vertices.end(), topMesh.Vertices.begin(), topMesh.Vertices.end());

		// concat top, cylinder, bottom indices
		cylinderMesh.Indices.resize(faceCount * 3);
		cylinderMesh.Indices.insert(cylinderMesh.Indices.end(), bottomMesh.Indices.begin(), bottomMesh.Indices.end());
		cylinderMesh.Indices.insert(cylinderMesh.Indices.end(), topMesh.Indices.begin(), topMesh.Indices.end());

		// prepare xz lerp
		vector<FVector> dists;
		vector<FVector> froms;
		froms.resize(circleSeg);
		dists.resize(circleSeg);
		for (UINT iCSeg = 0; iCSeg < circleSeg; ++iCSeg)
		{
			FVector vFrom = topMesh.Vertices[iCSeg].Position;
			FVector vTo = bottomMesh.Vertices[iCSeg].Position;

			dists[iCSeg] = vTo - vFrom;
			froms[iCSeg] = vFrom + (dists[iCSeg] * dCySeg); // cylinder calculation starts at 1 segment index so the interpolation starts after the first step
		}

		// calculate vertices
		for (UINT iCySeg = 0; iCySeg < cySeg; ++iCySeg)
		{
			// top to bottom
			float cyY = dY * (cySeg - iCySeg);

			for (UINT iCSeg = 0; iCSeg < circleSeg; ++iCSeg)
			{
				// set vertex
				FVector cyPos{};
				cyPos.X = froms[iCSeg].X + dists[iCSeg].X * iCySeg * dCySeg;
				cyPos.Z = froms[iCSeg].Z + dists[iCSeg].Z * iCySeg * dCySeg;
				cyPos.Y = cyY;

				cylinderMesh.Vertices[(iCySeg * circleSeg) + iCSeg].Position = cyPos;
			}
		}

		// calculate indices
		UINT index = 0;

		// concat with 'top circle'
		{
			for (UINT iCSeg = 0; iCSeg < circleSeg - 1; ++iCSeg)
			{
				// face 1 ABC
				cylinderMesh.Indices[index] = tcIndexBias + iCSeg;
				cylinderMesh.Indices[index + 1] = tcIndexBias + iCSeg + 1;
				cylinderMesh.Indices[index + 2] = iCSeg;

				// face 2 CBD						    
				cylinderMesh.Indices[index + 3] = iCSeg;
				cylinderMesh.Indices[index + 4] = tcIndexBias + iCSeg + 1;
				cylinderMesh.Indices[index + 5] = iCSeg + 1;
				index += 6;
			}

			// connect end with begin
			// face 1 ABC
			cylinderMesh.Indices[index] = tcIndexBias + (circleSeg - 1);				// end
			cylinderMesh.Indices[index + 1] = tcIndexBias;								// begin
			cylinderMesh.Indices[index + 2] = circleSeg - 1;							// end

			// face 2 CBD						    
			cylinderMesh.Indices[index + 3] = circleSeg - 1;							// end
			cylinderMesh.Indices[index + 4] = tcIndexBias;								// begin
			cylinderMesh.Indices[index + 5] = 0;										// begin
			index += 6;
		}
		
		// calculate cylinder indices
		for (UINT iCySeg = 0; iCySeg < cySeg; ++iCySeg)
		{
			for (UINT iCSeg = 0; iCSeg < circleSeg - 1; ++iCSeg)
			{
				// face 1 ABC
				cylinderMesh.Indices[index] = iCySeg * circleSeg + iCSeg;
				cylinderMesh.Indices[index + 1] = iCySeg * circleSeg + iCSeg + 1;
				cylinderMesh.Indices[index + 2] = (iCySeg + 1) * circleSeg + iCSeg;

				// face 2 CBD						    
				cylinderMesh.Indices[index + 3] = (iCySeg + 1) * circleSeg + iCSeg;
				cylinderMesh.Indices[index + 4] = iCySeg * circleSeg + iCSeg + 1;
				cylinderMesh.Indices[index + 5] = (iCySeg + 1) * circleSeg + iCSeg + 1;
				index += 6;
			}

			// connect end with begin
			// face 1 ABC
			cylinderMesh.Indices[index] = iCySeg * circleSeg + (circleSeg - 1);				// end
			cylinderMesh.Indices[index + 1] = iCySeg * circleSeg;							// begin
			cylinderMesh.Indices[index + 2] = (iCySeg + 1) * circleSeg + (circleSeg - 1);	// end

			// face 2 CBD						    
			cylinderMesh.Indices[index + 3] = (iCySeg + 1) * circleSeg + (circleSeg - 1);	// end
			cylinderMesh.Indices[index + 4] = iCySeg * circleSeg;							// begin
			cylinderMesh.Indices[index + 5] = (iCySeg + 1) * circleSeg;						// begin
			index += 6;
		}
		// The concatenation with the 'bottom circle' 
		// is processed in the last loop of the index calculation
	}
#pragma endregion

	outMesh = cylinderMesh;
}

void GeometryGenerator::CreateCircle(Mesh& outMesh, float radius, UINT segment)
{
	assert(segment >= 3);

	UINT vertexCount = segment + 1;
	float dRadian = -Radian(360.0f / segment); // multiply '-1' for clock wise
	outMesh.Vertices.resize(vertexCount);

	UINT index = 0;
	outMesh.Indices.resize(segment * 3);

	float dSeg = 1.0f / segment;

	// Zero Vertex 위치 ( 원의 중심에 대한 vertex 위치 )

	outMesh.Vertices[segment] = Vertex(FVector(0.0f), FVector(1.0f, 1.0f, 1.0f));
	for (UINT iSeg = 0; iSeg < segment; ++iSeg)
	{
		// calculate vertex
		float radian = dRadian * iSeg;
		outMesh.Vertices[iSeg] = Vertex(FVector{ cosf(radian) * radius, 0.0f, sinf(radian) * radius }, FVector{ 1.0f,1.0f,1.0f });

		// calculate index
		outMesh.Indices[index] = iSeg;
		outMesh.Indices[index + 1] = iSeg + 1;
		outMesh.Indices[index + 2] = segment; // center
		index += 3;
	}

	// 원은 끝과 처음이 이어져 있기 때문에 마지막 (i + 1) 위치의
	// 정점 인덱스를 0로 설정해줍니다.
	outMesh.Indices[index - 2] = 0;
}
