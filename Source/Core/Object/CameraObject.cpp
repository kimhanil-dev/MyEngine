#include "pch.h"
#include "CameraObject.h"

#include "Core/Render/Graphics/IGraphics.h"

#include "Utill/console.h"

void CameraObject::Init(IGraphics* renderer)
{
	renderer->BindCameraObject(this);

	mPosition.z = -50.0f;
}

void CameraObject::BindKeyInput(InputManager* inputManager)
{
	inputManager->BindInput('A', InputManager::KeyState::Press, [this](uint key, InputManager::KeyState keyState) {OnInputEvent(key, keyState); });
	inputManager->BindInput('D', InputManager::KeyState::Press, [this](uint key, InputManager::KeyState keyState) {OnInputEvent(key, keyState); });

	// camera move
	/*inputManager->BindInput_MousePos([this](const InputManager::MousePos& mousePos) {
		static InputManager::MousePos prevMousePos;
		const static float CAMERA_MOVE_SPEED = 1.0f / 400;
		if (prevMousePos.X == 0 && prevMousePos.Y == 0)
		{
			prevMousePos = mousePos;
		}

		float deltaX = (mousePos.X - prevMousePos.X) * CAMERA_MOVE_SPEED;
		float deltaY = (mousePos.Y - prevMousePos.Y) * CAMERA_MOVE_SPEED;

		mForwardRotation.y += deltaX;
		mForwardRotation.x += deltaY;

		prevMousePos = mousePos;
		});*/
}

void CameraObject::OnInputEvent(uint key, InputManager::KeyState keyState)
{
	switch (key)
	{
	case 'D':
	{
		XMFLOAT3 rightVector = GetRightVector();
		XMVECTOR rightVectorSIMD;
		rightVectorSIMD = XMLoadFloat3(&rightVector);

		XMVECTOR positionSIMD = XMLoadFloat3(&mPosition);

		positionSIMD += XMVectorScale(rightVectorSIMD, 0.1f);
		XMStoreFloat3(&mPosition, positionSIMD);
	}
		break;
	case 'A':
	{
		XMFLOAT3 rightVector = GetRightVector();
		XMVECTOR rightVectorSIMD;
		rightVectorSIMD = XMLoadFloat3(&rightVector);

		XMVECTOR positionSIMD = XMLoadFloat3(&mPosition);

		positionSIMD += XMVectorScale(rightVectorSIMD, -0.1f);
		XMStoreFloat3(&mPosition, positionSIMD);
	}
		break;
	default:
		break;
	}
}
