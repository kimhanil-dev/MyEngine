#include "pch.h"
#include "GeometryGenerator.h"

#include <math.h>
#include <algorithm>
#include <array>

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
			outMesh.Vertices[index].Position = XMFLOAT3(x, 0.0f, z);
			outMesh.Vertices[index].Normal = XMFLOAT3(0.0f,1.0f,0.0f);
			outMesh.Vertices[index].UV = XMFLOAT2(k * du, i * dv);
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
	std::ranges::for_each(topMesh.Vertices, [height](Vertex& vertex) { vertex.Position.y += height; }); // add height
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
	vector<XMVECTOR> dists;
	vector<XMVECTOR> froms;
	froms.resize(circleSeg);
	dists.resize(circleSeg);
	for (UINT iCSeg = 0; iCSeg < circleSeg; ++iCSeg)
	{
		XMVECTOR vFrom = XMLoadFloat3(&topMesh.Vertices[iCSeg + topCircleCenterVtxBias].Position);
		XMVECTOR vTo = XMLoadFloat3(&bottomMesh.Vertices[iCSeg].Position);

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
			XMVECTOR cyPos{};
			cyPos = froms[iCSeg] + dists[iCSeg] * (float)iCySeg * dCySeg;
			cyPos.m128_f32[3] = cyY;

			UINT index = iCyVtxStart + (iCySeg * circleSeg) + iCSeg;
			XMStoreFloat3(&cylinderMesh.Vertices[index].Position, cyPos);
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

void GeometryGenerator::CreateGeosphere(const XMFLOAT4& color, float radius, UINT numSubdivisions, Mesh& outMesh)
{
	const float x = 0.525731f;
	const float z = 0.850651f;

	array<XMFLOAT3, 12> vtxPoses = 
	{
		XMFLOAT3(-x,0.0f,z), XMFLOAT3(x,0.0f,z),
		XMFLOAT3(-x,0.0f,-z),XMFLOAT3(x,0.0f,-z),
		XMFLOAT3(0.0f,z,x),  XMFLOAT3(0.0f,z,-x),
		XMFLOAT3(0.0f,-z,x), XMFLOAT3(0.0f,-z,-x),
		XMFLOAT3(z,x,0.0f),  XMFLOAT3(-z,x,0.0f),
		XMFLOAT3(z,-x,0.0f), XMFLOAT3(-z,-x,0.0f),
	};

	size_t vtxCount = vtxPoses.size();

	outMesh.Vertices.reserve(vtxCount);
	ranges::for_each(vtxPoses, [&outMesh](const XMFLOAT3& v) {
		outMesh.Vertices.push_back({ v,{0.0f,0.0f,0.0f}, {0.0f,0.0f} });
		});

	outMesh.Indices = {
		1,4,0, 4,9,0, 4,5,9, 8,5,4, 1,8,4,
		1,10,8, 10,3,8, 8,3,5, 3,2,5, 3,7,2,
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
		10,1,6, 11,0,9, 2,11,9, 5,2,9, 11,2,7
	};

	// calculate normal
	for (Vertex& v : outMesh.Vertices)
	{
		XMVECTOR vSIMD = XMLoadFloat3(&v.Position);
		XMVECTOR n = XMVector3Normalize(vSIMD);

		XMStoreFloat3(&v.Normal, n);
	}

	for (UINT i = 0; i < numSubdivisions; ++i)
	{
		Subdivide(outMesh);
	}

	// make sphere (normalized position * radius)
	for (Vertex& v : outMesh.Vertices)
	{
		XMVECTOR vSIMD = XMLoadFloat3(&v.Position);
		XMVECTOR n = XMVector3Normalize(vSIMD);

		XMStoreFloat3(&v.Position, n * radius);
	}
}


static void Interpolate(const Vertex& from, const Vertex& to, float delta, Vertex& out)
{
	XMVECTOR fromPos = XMLoadFloat3(&from.Position);
	XMVECTOR toPos = XMLoadFloat3(&to.Position);
	XMVECTOR pos = fromPos + (toPos - fromPos) * delta;

	XMVECTOR fromNormal = XMLoadFloat3(&from.Normal);
	XMVECTOR toNormal = XMLoadFloat3(&to.Normal);
	XMVECTOR normal = (fromNormal * (1.0f - delta)) + (toNormal * (delta));
	normal = XMVector3Normalize(normal);

	XMStoreFloat3(&out.Position, pos);
	XMStoreFloat3(&out.Normal, normal);
}

void GeometryGenerator::Subdivide(Mesh& outMesh)
{
	UINT prevFaceCount = (UINT)outMesh.Indices.size() / 3;
	UINT faceCount = prevFaceCount * 4;	// 1개의 페이스는 4개의 페이스로 분리됨
	UINT vertexCount = prevFaceCount * 3 * 2;	 // 기존 face당 3개 증가

	vector<Vertex> subVertices;
	subVertices.resize(vertexCount);

	vector<UINT> subIndices;
	subIndices.reserve(faceCount * 3);

	UINT index = 0;
	UINT vtxIndex = 0;
	UINT subIndex = 0;
	for (UINT iFace = 0; iFace < prevFaceCount; ++iFace)
	{
		// get tri vertices
		index = iFace * 3;

		Vertex vtx0 = outMesh.Vertices[outMesh.Indices[index]];
		Vertex vtx1 = outMesh.Vertices[outMesh.Indices[index + 1]];
		Vertex vtx2 = outMesh.Vertices[outMesh.Indices[index + 2]];

		subIndex = iFace * 6;

		subVertices[subIndex] = vtx0;
		subVertices[subIndex + 2] = vtx1;
		subVertices[subIndex + 4] = vtx2;
		Interpolate(vtx0, vtx1, 0.5f, subVertices[subIndex + 1]);
		Interpolate(vtx1, vtx2, 0.5f, subVertices[subIndex + 3]);
		Interpolate(vtx2, vtx0, 0.5f, subVertices[subIndex + 5]);

		subIndices.push_back(subIndex);
		subIndices.push_back(subIndex + 1);
		subIndices.push_back(subIndex + 5);

		subIndices.push_back(subIndex + 5);
		subIndices.push_back(subIndex + 3);
		subIndices.push_back(subIndex + 4);

		subIndices.push_back(subIndex + 5);
		subIndices.push_back(subIndex + 1);
		subIndices.push_back(subIndex + 3);

		subIndices.push_back(subIndex + 1);
		subIndices.push_back(subIndex + 2);
		subIndices.push_back(subIndex + 3);
	}

	outMesh.Vertices = move(subVertices);
	outMesh.Indices = move(subIndices);
}
//
//void GeometryGenerator::Subdivide(Mesh& outMesh)
//{
//	UINT prevFaceCount = (UINT)outMesh.Indices.size() / 3;
//	UINT faceCount = prevFaceCount * 4;	// 1개의 페이스는 4개의 페이스로 분리됨
//	UINT vertexCount = prevFaceCount * 3;	 // 기존 face당 3개 증가
//
//	vector<XMVECTOR> subVertices;
//	subVertices.reserve(vertexCount);
//
//	vector<UINT> subIndices;
//	subIndices.reserve(faceCount * 3);
//
//	XMVECTOR vm0, vm1, vm2;
//	XMVECTOR v0, v1, v2;
//	UINT index = 0;
//	UINT subIndex = 0;
//	for (UINT iFace = 0; iFace < prevFaceCount; ++iFace)
//	{
//		// get tri vertices
//		index = iFace * 3;
//		v0 = XMLoadFloat3(&outMesh.Vertices[outMesh.Indices[index]].Position);
//		v1 = XMLoadFloat3(&outMesh.Vertices[outMesh.Indices[index + 1]].Position);
//		v2 = XMLoadFloat3(&outMesh.Vertices[outMesh.Indices[index + 2]].Position);
//
//		// calculate middle vertex position
//		vm0 = v0 + (v1 - v0) * 0.5f;
//		vm1 = v1 + (v2 - v1) * 0.5f;
//		vm2 = v2 + (v0 - v2) * 0.5f;
//
//		// make sub triangles
//		subVertices.push_back(v0);
//		subVertices.push_back(vm0);
//		subVertices.push_back(v1);
//		subVertices.push_back(vm1);
//		subVertices.push_back(v2);
//		subVertices.push_back(vm2);
//
//		subIndex = iFace * 6;
//		subIndices.push_back(subIndex);
//		subIndices.push_back(subIndex + 1);
//		subIndices.push_back(subIndex + 5);
//
//		subIndices.push_back(subIndex + 5);
//		subIndices.push_back(subIndex + 3);
//		subIndices.push_back(subIndex + 4);
//
//		subIndices.push_back(subIndex + 5);
//		subIndices.push_back(subIndex + 1);
//		subIndices.push_back(subIndex + 3);
//
//		subIndices.push_back(subIndex + 1);
//		subIndices.push_back(subIndex + 2);
//		subIndices.push_back(subIndex + 3);
//	}
//
//
//	// set outMesh
//	size_t vtxBufferSize = subVertices.size();
//	outMesh.Vertices.resize(vtxBufferSize);
//	for (size_t i = 0; i < vtxBufferSize; ++i)
//	{
//		XMStoreFloat3(&outMesh.Vertices[i].Position, subVertices[i]);
//	}
//
//	outMesh.Indices = move(subIndices);
//}

void GeometryGenerator::CreateCircle(Mesh& outMesh, float radius, UINT segment, bool isCenterAtStart)
{
	assert(segment >= 3);

	UINT vertexCount = segment + 1;
	
	float dRadian = XM_2PI / segment;

	//float dRadian = RadianF(360.0f / segment);
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

	outMesh.Vertices[centerIndex] = Vertex({0.0f,0.0f,0.0f});
	for (UINT iSeg = segStart; iSeg < segEnd; ++iSeg)
	{
		// calculate vertex
		float radian = dRadian * (iSeg - segStart);
		outMesh.Vertices[iSeg] = Vertex({ cosf(radian) * radius, 0.0f, sinf(radian) * radius });

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
