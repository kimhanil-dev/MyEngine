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

void GeometryGenerator::CreateCylinder(const float topRadius, const float bottomRadius, const float height, const UINT circleSeg, const UINT cylinderSeg, Mesh& outMesh)
{
	//***
// vertex order is `Vertices[row][column]`
//***  merge된 vertex vector와 index vector의 순서는 
//top circle,
//cylinder, 
//bottom circle 순 입니다.


	Mesh topMesh;
	CreateCircle(topMesh, topRadius, circleSeg, true);
	Mesh bottomMesh;
	CreateCircle(bottomMesh, bottomRadius, circleSeg);
	Mesh cylinderMesh; // TODO : outMesh 직접 사용하는 것과 마지막에 복사해서 전달하는 것의 속도차이 확인하기
	// 또한 RVO를 사용해보고 속도차이 확인하기
	// 또한 move 속도차이

	// circle has a center vertex (center of circle), 
	// Therefore, the vertex layout isn't aligned (vertex(col,row) can't be accessed using [row_num * max_col + col_num]).
	// bottom circle is ok, because center vertex in end of vertex buffer
	// However, top circle's center vertex is at the start of vertex buffer,
	// Therefore,	 vertex accessing method is must be biased ( + 1 {center vtx}})
	const UINT topCircleCenterVtxBias = 1;

	UINT cyVtxSegCount = (cylinderSeg - 2); // cylinder의 top과 bottom의 vertex는 topMesh, bottomMesh와 중복이므로 감소 시킵니다.
	UINT cyVtxCount = (UINT)cyVtxSegCount * circleSeg;
	UINT tcVtxCount = (UINT)topMesh.Vertices.size();

	// merge를 위해 bottom의 index를 변형하고, topMesh vertices를 height만큼 높입니다.
	UINT bcIndexBias = tcVtxCount + cyVtxCount;
	std::ranges::for_each(topMesh.Vertices, [height](Vertex& vertex) { vertex.Position.Y += height; }); // add height
	std::ranges::for_each(bottomMesh.Indices, [bcIndexBias](UINT& index) { index += bcIndexBias; }); // add index bias

	// concat top, cylinder, bottom vertices
	cylinderMesh.Vertices.resize(cyVtxCount);
	cylinderMesh.Vertices.insert(cylinderMesh.Vertices.begin(), topMesh.Vertices.begin(), topMesh.Vertices.end());
	cylinderMesh.Vertices.insert(cylinderMesh.Vertices.end(), bottomMesh.Vertices.begin(), bottomMesh.Vertices.end());

	// concat top, cylinder, bottom indices
	UINT faceCount = cylinderSeg * circleSeg * 2;// index는 cylinder의 top과 bottom을 topMesh와 bottomMesh로 연결해야 하므로 감소시키지 않습니다.

	cylinderMesh.Indices.resize(faceCount * 3);
	cylinderMesh.Indices.insert(cylinderMesh.Indices.begin(), topMesh.Indices.begin(), topMesh.Indices.end());
	cylinderMesh.Indices.insert(cylinderMesh.Indices.end(), bottomMesh.Indices.begin(), bottomMesh.Indices.end());


	// deltas
	float dY = height / (cylinderSeg - 1);	// cylinder seg 
	float dCySeg = 1.0f / (cylinderSeg - 1);

	// prepare xz lerp
	vector<FVector> dists;
	vector<FVector> froms;
	froms.resize(circleSeg);
	dists.resize(circleSeg);
	for (UINT iCSeg = 0; iCSeg < circleSeg; ++iCSeg)
	{
		FVector vFrom = topMesh.Vertices[iCSeg + topCircleCenterVtxBias].Position;
		FVector vTo = bottomMesh.Vertices[iCSeg].Position;

		dists[iCSeg] = vTo - vFrom;
		froms[iCSeg] = vFrom + (dists[iCSeg] * dCySeg); // cylinder calculation starts at 1 segment index so the interpolation starts after the first step
	}

	// calculate vertices
	UINT iCyVtxStart = tcVtxCount;
	for (UINT iCySeg = 0; iCySeg < cyVtxSegCount; ++iCySeg)
	{
		// top to bottom
		float cyY = dY * (cyVtxSegCount - iCySeg);

		for (UINT iCSeg = 0; iCSeg < circleSeg; ++iCSeg)
		{
			// set vertex
			FVector cyPos{};
			cyPos.X = froms[iCSeg].X + dists[iCSeg].X * iCySeg * dCySeg;
			cyPos.Z = froms[iCSeg].Z + dists[iCSeg].Z * iCySeg * dCySeg;
			cyPos.Y = cyY;

			cylinderMesh.Vertices[iCyVtxStart + (iCySeg * circleSeg) + iCSeg].Position = cyPos;
		}
	}

	// calculate cylinder indices
	{
		UINT cyIdxSegCount = cylinderSeg;
		UINT index = (UINT)topMesh.Indices.size();

		UINT cyRowVtxPos = 0;
		UINT cyNextRowVtxPos = 0;
		for (UINT iCySeg = 0; iCySeg < cyIdxSegCount - 1; ++iCySeg)
		{
			cyRowVtxPos = iCySeg * circleSeg + topCircleCenterVtxBias;
			cyNextRowVtxPos = cyRowVtxPos + circleSeg;
			for (UINT iCSeg = 0; iCSeg < circleSeg - 1; ++iCSeg)
			{
				// face 1 ABC
				cylinderMesh.Indices[index] = cyRowVtxPos + iCSeg;					// A ---- B
				cylinderMesh.Indices[index + 1] = cyRowVtxPos + iCSeg + 1;			// |  -  
				cylinderMesh.Indices[index + 2] = cyNextRowVtxPos + iCSeg;			// C-  
				// face 2 CBD						    
				cylinderMesh.Indices[index + 3] = cyNextRowVtxPos + iCSeg;			//      - B
				cylinderMesh.Indices[index + 4] = cyRowVtxPos + iCSeg + 1;			//    -   |
				cylinderMesh.Indices[index + 5] = cyNextRowVtxPos + iCSeg + 1;		// C------D 
				index += 6;
			}

			// cylinder 행의 마지막 버텍스는 그 행의 첫번째 버택스와 연결되어야 하는 특수함이 있으므로
			// 따로 처리해줍니다.
			// face 1 ABC
			cylinderMesh.Indices[index] = cyRowVtxPos + circleSeg - 1;
			cylinderMesh.Indices[index + 1] = cyRowVtxPos;
			cylinderMesh.Indices[index + 2] = cyNextRowVtxPos + circleSeg - 1;

			// face 2 CBD						    
			cylinderMesh.Indices[index + 3] = cyNextRowVtxPos + circleSeg - 1;
			cylinderMesh.Indices[index + 4] = cyRowVtxPos;
			cylinderMesh.Indices[index + 5] = cyNextRowVtxPos;

			index += 6;
		}
	}
	outMesh = move(cylinderMesh);
}

void GeometryGenerator::CreateSphere(const float radius, UINT segment, Mesh& outMesh)
{
	UINT vertexCount = segment * segment * 0.5f - segment + 2;
	UINT face = (segment - 2) * segment;
	UINT indexCount = face * 3;
	outMesh.Vertices.resize(vertexCount);
	outMesh.Indices.resize(indexCount);

	float dAngle = RadianF(360.0f) / segment;

	UINT ySegNum = segment * 0.5f - 1;

	vector<float> yPoses;
	yPoses.resize(ySegNum);
	vector<float> ratios;
	ratios.resize(ySegNum);

	for (int i = 0; i < ySegNum; ++i)
	{
		yPoses[i] = (cosf((i + 1)* dAngle));
		ratios[i] = (sinf((i + 1)* dAngle));
	}

	// vertex
	UINT index = 0;
	UINT vtxRowPos = 0;
	UINT vtxNextRowPos = 0;

	float xzAngle = 0.0f;
	for (UINT i = 0; i < segment; ++i)
	{
		float x = cosf(xzAngle);
		float z = sinf(xzAngle);

		// ignore top, bottom vertex
		vtxRowPos = i * ySegNum;
		vtxNextRowPos = vtxRowPos + ySegNum;
		for (UINT k = 0; k < ySegNum; ++k)
		{
			//vertex
			FVector pos = FVector(x * ratios[k], yPoses[k], z * ratios[k]);
			pos *= radius;

			outMesh.Vertices[i * ySegNum + k].Position = pos;
			outMesh.Vertices[i * ySegNum + k].Color = { 1.0f,1.0,1.0f };
		}

		xzAngle += dAngle;
	}

	for (UINT i = 0; i < segment - 1; ++i)
	{
		vtxRowPos = i * ySegNum;
		vtxNextRowPos = vtxRowPos + ySegNum;
		for (UINT k = 0; k < segment / 4; ++k)
		{
			// index
			outMesh.Indices[index] = vtxRowPos + k;
			outMesh.Indices[index + 1] = vtxNextRowPos + k;
			outMesh.Indices[index + 2] = vtxRowPos + k + 1;

			outMesh.Indices[index + 3] = vtxRowPos + k + 1;
			outMesh.Indices[index + 4] = vtxNextRowPos + k;
			outMesh.Indices[index + 5] = vtxNextRowPos + k + 1;
			index += 6;
		}
	}

	//top
	outMesh.Vertices[vertexCount - 2].Position = FVector(0.0f, radius, 0.0f);
	for (UINT i = 0; i < segment - 1; ++i)
	{
		// index
		outMesh.Indices[index] = vertexCount - 2;
		outMesh.Indices[index + 1] = (i + 1) * ySegNum;
		outMesh.Indices[index + 2] = i * ySegNum;

		index += 3;
	}

	//bottom
	outMesh.Vertices[vertexCount - 1].Position = FVector(0.0f, -radius, 0.0f);
	for (UINT i = 0; i < segment - 1; ++i)
	{
		// index
		outMesh.Indices[index] = vertexCount - 1;
		outMesh.Indices[index + 1] = (i + 1) * ySegNum + (ySegNum - 1);
		outMesh.Indices[index + 2] = i * ySegNum + (ySegNum - 1);

		index += 3;
	}
}

void GeometryGenerator::CreateCircle(Mesh& outMesh, float radius, UINT segment, bool isCenterAtStart)
{
	assert(segment >= 3);

	UINT vertexCount = segment + 1;
	float dRadian = RadianF(360.0f / segment);
	outMesh.Vertices.resize(vertexCount);

	UINT index = 0;
	outMesh.Indices.resize(segment * 3);

	float dSeg = 1.0f / segment;

	// Zero Vertex 위치 ( 원의 중심에 대한 vertex 위치 )

	UINT centerIndex = segment;
	UINT segStart = 0;
	UINT segEnd = segment;
	if (isCenterAtStart)
	{
		centerIndex = 0;
		segStart = 1;
		segEnd = vertexCount;
	}

	outMesh.Vertices[centerIndex] = Vertex(FVector(0.0f), FVector(1.0f, 1.0f, 1.0f));
	for (UINT iSeg = segStart; iSeg < segEnd; ++iSeg)
	{
		// calculate vertex
		float radian = dRadian * (iSeg - segStart);
		outMesh.Vertices[iSeg] = Vertex(FVector{ cosf(radian) * radius, 0.0f, sinf(radian) * radius }, FVector{ 1.0f,1.0f,1.0f });

		// calculate index
		// cosf, sinf를 이용해 그린 원은 반시계 방향으로 그려지므로
		// 삼각형을 역순으로 만듭니다.
		// ABC -> BAC
		outMesh.Indices[index] = iSeg + 1;
		outMesh.Indices[index + 1] = iSeg;
		outMesh.Indices[index + 2] = centerIndex; // center
		index += 3;
	}

	// 원의 마지막 버텍스는 시작 버텍스와 이어져야 하므로,
	// 삼각형 BAC를 (B(start), A(end), 원의 중심)으로 설정해주기 위해,
	// B 인덱스만 수정해줍니다.

	// index - (원의 중심 버텍스, A의 버텍스, 배열은 0번부터 시작) == index - 3
	if (isCenterAtStart)
	{
		outMesh.Indices[index - 3] = 1;	// 버퍼의 첫 번째는 원의 중심 버텍스이기 때문에, +1
	}
	else
	{
		outMesh.Indices[index - 3] = 0; // 원의 중심 버텍스가, 버텍스 버퍼의 마지막에 있기 때문에 0번을 지정해도 상관 X
	}
}
