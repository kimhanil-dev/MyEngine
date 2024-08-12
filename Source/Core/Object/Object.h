#pragma once

#include "IObject.h"

#include <DirectXMath.h>

class IGeometryModifier;

class Object : public IObject
{
public:
	Object();
	virtual ~Object();

	// IObject을(를) 통해 상속됨
	void Update(float deltaTime) override;

protected:
	weak_ptr<IGeometryModifier> mMesh;

	XMFLOAT3 mPosition = {0.0f,0.0f,0.0f};
	XMFLOAT3 mForwardRotation = {0.0f,0.0f,0.0f};
	XMFLOAT3 mDeferredRotation = {0.0f,0.0f,0.0f};

	XMFLOAT4X4 GetWorldTransform() const;

	// IObject을(를) 통해 상속됨
	void Init(IGraphics* const renderer) override;
	void BindKeyInput(InputManager* const inputManager) override;
	void Destroy() override;

	const XMFLOAT3& GetPosition() const override;
	const XMFLOAT3& GetRotation() const override;
	const XMFLOAT3 GetUpVector() const override;

	// IObject을(를) 통해 상속됨
	const XMFLOAT3 GetForwardVector() const override;

	// IObject을(를) 통해 상속됨
	const XMFLOAT3 GetRightVector() const override;
};
