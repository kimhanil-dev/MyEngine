#pragma once

#include <vector>

#include "Window/framework.h"
#include "Core/Types.h"
#include "Core/Math/Vector.h"
#include "Vertex.h"

class Object;

static constexpr float PI = 3.14159265359f;
#define Radian(Degree) Degree * (PI / 180.0f)

namespace Render
{
	void Init(const HWND hWnd, const uint width, const uint height);
	void Render(const Object* camera, const std::vector<Object*> objects);
	void Release();
}

