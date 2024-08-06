#pragma once

#include <vector>
#include "Core/Render/Vertex.h"

struct Mesh;

class Object
{
public:
	Object() {}

public:
	int mID = 0;

	Mesh* mMesh = nullptr;

	std::vector<Vertex> mVertices;
};

