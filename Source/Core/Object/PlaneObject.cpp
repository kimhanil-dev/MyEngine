#include "pch.h"
#include "PlaneObject.h"

#include "Core/Render/Graphics/IGraphics.h"

#include "Core/Render/GeometryGenerator.h"

void PlaneObject::Init(IGraphics* renderer)
{
	GeometryGenerator geoGen;

	Mesh planeMesh;
	geoGen.CreateGrid(500.0f, 500.0f, 300, 300, planeMesh);
	mMesh = renderer->BindMesh(&planeMesh);

	mPosition.x = 100.0f;
	mForwardRotation.z = 45.0f;
}

void PlaneObject::Update(float deltaTime)
{
	Object::Update(deltaTime);
}
