#pragma once
#include <vector>

#include "Core/Math/Vector.h"
#include "Core/Render/Vertex.h"

struct Mesh;

class Object
{
public:
	Object() {}
	Object(int id, FVector origin, FVector rotate, Mesh* mesh) : mID(id), mOrigin(origin), mRotate(rotate), mMesh(mesh) {}
	Object(int id ,FVector origin, FVector rotate, std::vector<Vertex> vertices) : mID(id), mOrigin(origin), mRotate(rotate), mVertices(vertices) {}
	Object(int id ,FVector origin, FVector rotate) : mID(id), mOrigin(origin), mRotate(rotate) {}
public:
	int mID = 0;
	FVector	mOrigin;
	FVector mRotate;

	Mesh* mMesh = nullptr;

	std::vector<Vertex> mVertices;
};

