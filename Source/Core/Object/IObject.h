#pragma once

#include <vector>
#include <memory>
#include <DirectXMath.h>

using namespace DirectX;
using namespace std;

class IGraphics;
class InputManager;

class IObject
{
public:
	virtual void Init(IGraphics* const renderer) = 0;
	virtual void BindKeyInput(InputManager* const inputManager) = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Destroy() = 0;

	virtual const XMFLOAT3 GetForwardVector() const = 0;
	virtual const XMFLOAT3 GetUpVector() const = 0;
	virtual const XMFLOAT3 GetRightVector() const = 0;
	virtual const XMFLOAT3& GetPosition() const = 0;
	virtual const XMFLOAT3& GetRotation() const = 0;
};

