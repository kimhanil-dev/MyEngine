#include "pch.h"
#include "PlaneObject.h"

#include "Core/Render/Graphics/IGraphics.h"

#include "Core/Render/GeometryGenerator.h"
#include "Core/Render/Graphics/IGeometryModifier.h"

void PlaneObject::Init(IGraphics* renderer)
{
	GeometryGenerator geoGen;

	Mesh planeMesh;
	geoGen.CreateGrid(30.0f, 30.0f, 300, 300, planeMesh);

	// 
	{
		for (auto& vertex : planeMesh.Vertices)
		{
			vertex.Position.y = ( - (vertex.Position.x * vertex.Position.x) - (vertex.Position.z * vertex.Position.z) + 3) / 25.0f;

			XMFLOAT3 n(2 * (vertex.Position.x), 1, 2 * (vertex.Position.z) );
			XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
			XMStoreFloat3(&vertex.Normal, unitNormal);
		}
	}

	mMesh = renderer->BindMesh(&planeMesh);

	Material material;
	material.Diffuse = { 0.3f, 0.3f, 1.0f, 1.0f };
	material.Ambient = { 0.3f, 0.3f, 1.0f, 1.0f };
	material.Specular = { 1.0f, 1.0f, 1.0f, 2.0f };

	mMesh.lock()->SetRaw("gMaterial", &material, sizeof(material));

	mPosition.y = 0.0f;
	mPosition.z = 0.0f;
	//mPosition.x = 100.0f;
	//mForwardRotation.z = 45.0f;
}

void PlaneObject::Update(float deltaTime)
{
	Object::Update(deltaTime);

	static float rotation = 0.0f;
	rotation += deltaTime * 1.0f;

	mForwardRotation.y = XMConvertToRadians(XMConvertToDegrees(rotation));

	if (rotation >= 360.0f)
	{
		rotation -= 360.0f;
	}

	mDeferredRotation = mForwardRotation;

	/*if (!mMesh.expired())
	{
		mLocalTotalTime += deltaTime;
		mMesh.lock()->SetFloat("gTime", mLocalTotalTime);
	}*/
}
