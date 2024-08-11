#pragma once

#include <DirectXMath.h>

using namespace DirectX;

struct Material
{
	XMFLOAT4 Ambient = { 0.0f,0.0f,0.0f,0.0f };
	XMFLOAT4 Diffuse = { 0.0f,0.0f,0.0f,0.0f };
	XMFLOAT4 Specular = { 0.0f,0.0f,0.0f,0.0f }; // w = SpecPower
	XMFLOAT4 Reflect = { 0.0f,0.0f,0.0f,0.0f };
};