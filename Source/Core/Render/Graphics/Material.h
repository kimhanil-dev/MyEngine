#pragma once

#include <DirectXMath.h>

using namespace DirectX;

struct Material
{
	XMFLOAT4 Ambient = {0.0f,0.0f,0.0f,0.0f};
	XMFLOAT4 Diffuse = { 0.0f,0.0f,0.0f,0.0f };
	XMFLOAT4 Specular = { 0.0f,0.0f,0.0f,0.0f };
};

struct DirectionLight : Material
{
	XMFLOAT4 Direction = { 0.0f,0.0f,0.0f,0.0f };
};

struct PointLight : Material
{
	XMFLOAT4 Position = { 0.0f,0.0f,0.0f,0.0f };	// float3 Position, float Range
	XMFLOAT4 Att = { 0.0f,0.0f,0.0f,0.0f };		// float3 Att, float pad
};

struct SpotLight : PointLight
{
	XMFLOAT4 Direction = { 0.0f,0.0f,0.0f,0.0f }; // float3 Direction, float Spot( pow count )
};

