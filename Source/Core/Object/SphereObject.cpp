#include "pch.h"
#include "SphereObject.h"

#include "Core/Render/Graphics/IGraphics.h"
#include "Core/Render/Graphics/IGeometryModifier.h"

#include "Core/Render/GeometryGenerator.h"

void SphereObject::Init(IGraphics* const renderer)
{
	GeometryGenerator geoGen;

	Mesh geoSphere;
	geoGen.CreateGeosphere({ 0.5f,1.0f,1.0f,1.0f }, 10.0f, 4, geoSphere);

	mMesh = renderer->BindMesh(&geoSphere);
}

void SphereObject::Update(float deltaTime)
{
	Object::Update(deltaTime);
}

void SphereObject::BindKeyInput(InputManager* const inputManager)
{
}

void SphereObject::Destroy()
{
}


