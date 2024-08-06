#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include <DirectXMath.h>

#include "Core/Types.h"
#include "Vertex.h"

using namespace std;

struct Mesh
{
	vector<Vertex> Vertices;
	vector<UINT> Indices;
};