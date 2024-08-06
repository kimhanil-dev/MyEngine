#pragma once

#include <memory>

#include "Core/Render/Mesh.h"
#include "Core/Types.h"

namespace fbx
{
	shared_ptr<Mesh> GetMesh(uint id);
	void LoadFBX(const char* path, uint id);
	void Release();
}

