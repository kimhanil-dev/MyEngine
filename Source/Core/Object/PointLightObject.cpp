#include "pch.h"
#include "PointLightObject.h"

#include "Core/Render/Graphics/IGraphics.h"
#include "Core/Render/GeometryGenerator.h"

void PointLightObject::Init(IGraphics* const renderer)
{
	GeometryGenerator geoGen;

	Mesh geoSphere;
	geoGen.CreateGeosphere({ 1.0f,0.0f,0.0f,1.0f}, 2.0f, 6, geoSphere);

	mMesh = renderer->BindMesh(&geoSphere);

	PointLight plDesc;

	plDesc.Color = { 1.0f,1.0f,1.0f };
	plDesc.Intensity = 1.0f;
	plDesc.Radius = 1000.0f;

	renderer->BindPointLight(this, plDesc);

	mPosition.z = 30.0f;
}

void PointLightObject::Update(float deltaTime)
{

	static float rotation = 0.0f;
	rotation += 20.0f * deltaTime;

	mForwardRotation.y = XMConvertToRadians(rotation);

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
