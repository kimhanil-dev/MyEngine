#include "pch.h"
#include "PlaneObject.h"

#include "Core/Render/Graphics/IGraphics.h"

#include "Core/Render/GeometryGenerator.h"
#include "Core/Render/Graphics/IGeometryModifier.h"

void PlaneObject::Init(IGraphics* renderer)
{
	GeometryGenerator geoGen;

	Mesh planeMesh;
	geoGen.CreateGrid(500.0f, 500.0f, 300, 300, planeMesh);

	mMesh = renderer->BindMesh(&planeMesh);

	Material material;
	material.Diffuse = { 0.3f, 0.3f, 1.0f, 1.0f };
	material.Ambient = { 0.3f, 0.3f, 1.0f, 1.0f };
	material.Specular = { 1.0f, 1.0f, 1.0f, 2.0f };

	mMesh.lock()->SetRaw("gMaterial", &material, sizeof(material));

	mPosition.x = 100.0f;
	mForwardRotation.z = 45.0f;
}

void PlaneObject::Update(float deltaTime)
{
	Object::Update(deltaTime);

	if (!mMesh.expired())
	{
		mLocalTotalTime += deltaTime;
		mMesh.lock()->SetFloat("gTime", mLocalTotalTime);
	}
}
