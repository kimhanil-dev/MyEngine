#pragma once

#include "Core/Types.h"

struct Mesh;

namespace fbx
{
	Mesh* GetMesh(uint id);
	void LoadFBX(const char* path, uint id);
	void Release();
}

