#pragma once

#include <DirectXMath.h>

using namespace DirectX;

struct PointLight
{
	XMFLOAT3 Color;
	float Intensity;
	float Radius;
};