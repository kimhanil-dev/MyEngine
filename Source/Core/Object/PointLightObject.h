#pragma once

#include "Object.h"

class PointLightObject : public Object
{
public:

	// Object을(를) 통해 상속됨
	void Init(IGraphics* const renderer) override;
	void Update(float deltaTime) override;
	void BindKeyInput(InputManager* const inputManager) override;
	void Destroy() override;
};

