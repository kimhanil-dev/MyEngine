#pragma once
#include <vector>

#include "Core/Framwork/Application.h"
#include "Core/Render/Graphics/GraphicsFactory.h"

class IGraphics;
class Object;

class Game :
    public Application
{
public:
    Game(HINSTANCE hAppInst);
    virtual ~Game();

    bool Init() override;

protected:
    void DrawScene() override;
    void UpdateScene(float deltaTime) override;

    void LoadGame();

private:

    IGraphics* mRenderers[(uint)Renderer::Count];
    Renderer mRenderType = Renderer::DriectX;

    Object* mCamera = nullptr;
    vector<Object*> mObjects;

    // Inherited via FrameWork
    void OnResize() override;
};

