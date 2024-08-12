#pragma once
#include "Object.h"

#include "Core/Input/InputManager.h"

class CameraObject :
    public Object
{
public:

    virtual void Init(IGraphics* renderer) override;
    virtual void BindKeyInput(InputManager* inputManager) override;

protected:
    void MoveCamera();
    void OnInputEvent(unsigned int key, InputManager::KeyState keyState);
};

