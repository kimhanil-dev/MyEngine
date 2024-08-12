#pragma once
#include "Object.h"
class PlaneObject :
    public Object
{
public:

    void Init(IGraphics* renderer) override;
    void Update(float deltaTime) override;
};

