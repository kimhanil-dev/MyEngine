#include "pch.h"
#include "Game.h"

#include <array>

#include "Utill/console.h"
#include "Utill/fbx.h"
#include "Core/Object/Object.h"
#include "Core/Render/Graphics/IGraphics.h"
#include "Core/Input/InputManager.h"
#include "Core/Framwork/Timer.h"
#include "Core/Render/GeometryGenerator.h"
#include "Utill/PerformanceTester.h"

#include "Core/Object/SphereObject.h"
#include "Core/Object/PointLightObject.h"
#include "Core/Object/CameraObject.h"
#include "Core/Object/PlaneObject.h"

Game::Game(HINSTANCE hAppInst)
	:Application(hAppInst),
	mRenderer(GetRenderer(Renderer::DriectX))
{
}

Game::~Game()
{
	if (mCamera != nullptr)
	{
		delete mCamera;
		mCamera = nullptr;
	}

	for (int i = 0; i < mObjects.size(); ++i)
	{
		mObjects[i]->Destroy();
		delete mObjects[i];
	}
	mObjects.clear();

	fbx::Release();

	mRenderer->Release();
}

void Game::OnResize()
{
	Application::OnResize();

	if (mRenderer->IsInited())
		mRenderer->ResizeWindow(mClientWidth, mClientHeight);
}

bool Game::Init()
{
	if (!Application::Init())
		return false;

	if (!mRenderer->Init(mhMainWnd))
		return false;

	LoadGame();

	return true;
}

void Game::DrawScene()
{
	mRenderer->Render();
}

void Game::UpdateScene(float deltaTime)
{
	mInputManager->Update();

	for (IObject* obj : mObjects)
	{
		obj->Update(deltaTime);
	}
}

void Game::LoadGame()
{
	fbx::LoadFBX("./Resource/fbx/box.fbx", 0);
	fbx::LoadFBX("./Resource/fbx/dragon.fbx", 1);

	mInputManager->BindInput(VK_F1, InputManager::KeyState::Down, [this](unsigned int key, InputManager::KeyState state) {mCurrState = State::Destroy, SendMessage(mhMainWnd, WM_CLOSE, NULL, NULL); });
	mInputManager->BindInput(VK_F2, InputManager::KeyState::Down, [this](unsigned int key, InputManager::KeyState state) {mCurrState = mCurrState == State::Pause ? State::Idle : State::Pause; });

	/*const UINT ROW_COUNT = 6;
	const UINT COLUMN_COUNT = 6;
	const int WIDTH = 100;
	const int HEIGHT= 100;
	for (UINT i = 0; i < ROW_COUNT; ++i)
	{
		for (UINT j = 0; j < COLUMN_COUNT; ++j)
		{
			float x = -WIDTH * 0.5f + i * (WIDTH / COLUMN_COUNT);
			float y = -HEIGHT * 0.5f + j * (HEIGHT / ROW_COUNT);
			mObjects.push_back(new SphereObject({  x,  y,0.0f}));
		}
	}*/
	mObjects.push_back(new SphereObject({ 0.0f,0.0f,0.0f }));
	mObjects.push_back(new PointLightObject);
	mObjects.push_back(new CameraObject);
	mObjects.push_back(new PlaneObject);

	for (IObject* obj : mObjects)
	{
		obj->Init(mRenderer);
	}

	for (IObject* obj : mObjects)
	{
		obj->BindKeyInput(mInputManager.get());
	}
}