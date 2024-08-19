#include "pch.h"
#include "SphereObject.h"

#include "Core/Render/Graphics/IGraphics.h"
#include "Core/Render/Graphics/IGeometryModifier.h"

#include "Core/Render/GeometryGenerator.h"

#include "Core/Render/Graphics/Material.h"

void SphereObject::Init(IGraphics* const renderer)
{

	GeometryGenerator geoGen;

	Mesh geoSphere;
	geoGen.CreateGeosphere({ 0.5f,1.0f,1.0f,1.0f }, 10.0f, 3, geoSphere);

	Material material;
	material.Diffuse = { 0.3f, 0.3f, 1.0f, 1.0f };
	material.Ambient = { 0.3f, 0.3f, 1.0f, 1.0f };
	material.Specular = { 1.0f, 1.0f, 1.0f, 2.0f };

	mMesh = renderer->BindMesh(&geoSphere);
	mMesh.lock()->SetRaw("gMaterial", &material, sizeof(material));
}

void SphereObject::Update(float deltaTime)
{
	Object::Update(deltaTime);

	if (!mMesh.expired())
	{
		mLocalTotalTime += deltaTime;
		mMesh.lock()->SetFloat("gTime", mLocalTotalTime);
	}
}

void SphereObject::BindKeyInput(InputManager* const inputManager)
{
}

void SphereObject::Destroy()
{
}


