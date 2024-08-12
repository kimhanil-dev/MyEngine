#pragma once
#include "Object.h"
class SphereObject :
    public Object
{
public:
    SphereObject()
    {

    }

    SphereObject(XMFLOAT3 position)
    {
        mPosition = position;
    }
public:

    // Object을(를) 통해 상속됨
    void Init(IGraphics* const renderer) override;

    void Update(float deltaTime) override;

    void BindKeyInput(InputManager* const inputManager) override;

    void Destroy() override;

};

