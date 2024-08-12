#include "pch.h"
#include "Object.h"

#include "Core/Render/Graphics/IGeometryModifier.h"

#include "Core/Render/Graphics/IGraphics.h"

Object::Object()
{
}

Object::~Object()
{
}


void Object::Update(float deltaTime)
{
	if (!mMesh.expired())
	{
		mMesh.lock()->SetMatrix("gWorld", GetWorldTransform());
	}
}

XMFLOAT4X4 Object::GetWorldTransform() const
{
	XMFLOAT4X4 result;

	XMMATRIX world = XMMatrixIdentity();

	XMVECTOR forwardRot = XMLoadFloat3(&mForwardRotation);
	XMVECTOR deferredRot = XMLoadFloat3(&mDeferredRotation);

	XMMATRIX matFR = XMMatrixRotationRollPitchYawFromVector(forwardRot);
	XMMATRIX matTrans = XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);
	XMMATRIX matDR = XMMatrixRotationRollPitchYawFromVector(deferredRot);

	world = world * matFR * matTrans * matDR;

	XMStoreFloat4x4(&result, world);

	return result;
}

void Object::Init(IGraphics* const renderer)
{
}

void Object::BindKeyInput(InputManager* const inputManager)
{
}

void Object::Destroy()
{
}

const XMFLOAT3& Object::GetPosition() const
{
	return mPosition;
}

const XMFLOAT3& Object::GetRotation() const
{
	return mForwardRotation;
}

const XMFLOAT3 Object::GetUpVector() const
{
	XMFLOAT4X4 worldMat = GetWorldTransform();
	XMFLOAT3 upVector = { worldMat._21, worldMat._22, worldMat._23 };

	XMVECTOR upVectorSIMD = XMLoadFloat3(&upVector);
	upVectorSIMD = XMVector3Normalize(upVectorSIMD);

	XMStoreFloat3(&upVector, upVectorSIMD);
	return upVector;
}

const XMFLOAT3 Object::GetForwardVector() const
{
	XMFLOAT4X4 worldMat = GetWorldTransform();
	XMFLOAT3 forwardVector = { worldMat._31, worldMat._32, worldMat._33 };

	XMVECTOR forwardVectorSIMD = XMLoadFloat3(&forwardVector);
	forwardVectorSIMD = XMVector3Normalize(forwardVectorSIMD);

	XMStoreFloat3(&forwardVector, forwardVectorSIMD);

	return forwardVector;
}

const XMFLOAT3 Object::GetRightVector() const
{
	XMFLOAT4X4 worldMat = GetWorldTransform();
	XMFLOAT3 rightVector = { worldMat._11, worldMat._12, worldMat._13 };

	XMVECTOR rightVectorSIMD = XMLoadFloat3(&rightVector);
	rightVectorSIMD = XMVector3Normalize(rightVectorSIMD);

	XMStoreFloat3(&rightVector, rightVectorSIMD);

	return rightVector;
}
