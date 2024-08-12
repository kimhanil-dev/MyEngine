#pragma once
#include <vector>

#include "Core/Render/Mesh.h"
#include "Core/Framwork/Application.h"
#include "Core/Render/Graphics/GraphicsFactory.h"
#include "Core/Render/Graphics/IGeometryModifier.h"

class IGraphics;
class IGeometryModifier;
class IObject;

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
    vector<weak_ptr<IGeometryModifier>> mGeometryMods;

    IGraphics* mRenderer;
    Renderer mRenderType = Renderer::DriectX;

    IObject* mCamera = nullptr;
    vector<IObject*> mObjects;

    

    // Inherited via FrameWork
    void OnResize() override;
};

