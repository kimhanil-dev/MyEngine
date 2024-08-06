#pragma once
#include <DirectXMath.h>

using namespace DirectX;

struct Vertex
{
	XMFLOAT3 Position;
	XMFLOAT4 Color	= XMFLOAT4(1.0f,1.0f,1.0f,1.0f);
	XMFLOAT3 Normal;
	XMFLOAT2 UV;
};