#include "pch.h"
#include "PointLightObject.h"

#include "Core/Render/Graphics/IGraphics.h"
#include "Core/Render/Graphics/IGeometryModifier.h"
#include "Core/Render/GeometryGenerator.h"

#include "Core/Input/InputManager.h"

void PointLightObject::Init(IGraphics* const renderer)
{
	GeometryGenerator geoGen;

	Mesh geoSphere;
	geoGen.CreateGeosphere({ 1.0f,0.0f,0.0f,1.0f}, 2.0f, 6, geoSphere);

	mMesh = renderer->BindMesh(&geoSphere);

	Material material;
	material.Diffuse = { 0.3f, 0.3f, 1.0f, 1.0f };
	material.Ambient = { 0.3f, 0.3f, 1.0f, 1.0f };
	material.Specular = { 1.0f, 1.0f, 1.0f, 2.0f };

	mMesh.lock()->SetRaw("gMaterial", &material, sizeof(material));

	PointLight plDesc;
	plDesc.Position = { 0.0f,30.0f,0.0f,1000.0f };
	plDesc.Ambient = { 0.1f, 0.1f,0.1f,1.0f };
	plDesc.Diffuse = { 1.0f,1.0f,1.0f,1.0f };
	plDesc.Specular = { 0.3f,0.3f,0.3f,1.0f };
	plDesc.Att = { 0.1f,0.001f,0.001f, 0.0f};
	
	renderer->BindPointLight(this, plDesc);

	SpotLight slDesc;
	slDesc.Diffuse = { 0.6f,0.1f,0.1f,1.0f };
	slDesc.Specular = { 0.6f,0.1f,0.1f,1.0f };
	slDesc.Ambient = { 0.6f,0.1f,0.1f,1.0f };
	slDesc.Att = { 0.01f,0.0f,0.0f,0.0f };
	slDesc.Position = { -30.0f,0.0f,0.0f,1000.0f };
	slDesc.Direction = { 1.0f,0.0f,0.0f, 64.0f};

	renderer->BindSpotLight(this, slDesc);

	mPosition.z = 0.0f;
	mPosition.x = 30.0f;
}

void PointLightObject::Update(float deltaTime)
{

	static float rotation = 0.0f;
	rotation += deltaTime;

	mForwardRotation.y = XMConvertToRadians(XMConvertToDegrees(rotation));

	if (rotation >= 360.0f)
	{
		rotation -= 360.0f;
	}


	mDeferredRotation = mForwardRotation;

	
	Object::Update(deltaTime);
}


void PointLightObject::BindKeyInput(InputManager* const inputManager)
{
}

void PointLightObject::Destroy()
{
}
