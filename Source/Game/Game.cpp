#include "pch.h"
#include "Game.h"

#include "Utill/console.h"
#include "Utill/fbx.h"
#include "Core/Object/Object.h"
#include "Core/Render/Graphics/IGraphics.h"
#include "Core/Input/InputManager.h"
#include "Core/Framwork/Timer.h"

Game::Game(HINSTANCE hAppInst)
	:Application(hAppInst)
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
		delete mObjects[i];
	}

	fbx::Release();

	mObjects.clear();

	for (auto& renderer : mRenderers)
		renderer->Release();
}

void Game::OnResize()
{
	for (IGraphics* renderer : mRenderers)
	{
		if (renderer != nullptr)
			renderer->ResizeWindow(mClientWidth, mClientHeight);
	}
}

bool Game::Init()
{
	Application::Init();

	HRESULT hr = S_OK;

	// Init renderers
	mRenderers[(uint)Renderer::DriectX] = GetRenderer(Renderer::DriectX);
	mRenderers[(uint)Renderer::Software] = GetRenderer(Renderer::Software);

	for (auto& renderer : mRenderers)
	{
		if (FAILED(hr = renderer->Init(mhMainWnd)))
		{
			PrintError("Renderer init failed : %x\n", hr);
			assert(false);
		}
	}

	for (auto object : mObjects)
	{
		for (auto& renderer : mRenderers)
		{
			renderer->AddObject(object);
		}
	}

	// camera
	for (auto& renderer : mRenderers)
	{
		renderer->SetCamera(mCamera);
	}

	return true;
}

void Game::DrawScene()
{
	mRenderers[(uint)mRenderType]->Render();
}

void Game::UpdateScene(float deltaTime)
{
	constexpr float rotateSpeed = 50.0f;
	for (int i = 0; i < mObjects.size(); ++i)
	{
		mObjects[i]->mRotate += rotateSpeed * mTimer->DeltaTime();
	}
}

void Game::LoadGame()
{
	fbx::LoadFBX("./Resource/fbx/Box.fbx", 0);
	fbx::LoadFBX("./Resource/fbx/dragon.fbx", 1);

	Object* moveable = new Object(0, FVector(0.0f, 0.0f, 300.0f), 0.0f, fbx::GetMesh(1));

	mInputManager->BindInput(VK_F1, InputManager::KeyState::Down, [this](unsigned int key, InputManager::KeyState state) {mCurrState = State::Destroy, SendMessage(mhMainWnd, WM_CLOSE, NULL, NULL); });
	mInputManager->BindInput(VK_F2, InputManager::KeyState::Down, [this](unsigned int key, InputManager::KeyState state) {mCurrState = mCurrState == State::Pause ? State::Idle : State::Pause; });
	mInputManager->BindInput(VK_F3, InputManager::KeyState::Down, [this](unsigned int key, InputManager::KeyState state) {mRenderType = mRenderType == Renderer::DriectX ? Renderer::Software : Renderer::DriectX; });
	mInputManager->BindInput(VK_LEFT, InputManager::KeyState::Down, [moveable](unsigned int key, InputManager::KeyState state) {moveable->mOrigin.X -= 10.0f; });
	//mInputManager->BindInput_MousePos([](InputManager::MousePos pos) {Print("Mouse X{%d}:Y{%d}\n", pos.X, pos.Y); });

	mCamera = new Object(0, 0.0f, 0.0f);
	mObjects.push_back(moveable);

	// triangle
	mObjects.push_back(new Object(1, FVector(500.0f, 540.0f, 0.0f), 0.0f,
		{
		{FVector(0.0f,-50.0f,0.0f), FVector(1.0f,0.0f,0.0f)},
		{FVector(50.0f,50.0f,0.0f), FVector(0.0f,1.0f,0.0f) },
		{FVector(-50.0f,50.0f,0.0f), FVector(0.0f,0.0f,1.0f)},
		{FVector(0.0f,-50.0f,0.0f), FVector(0.0f)},
		}));

	mObjects.push_back(new Object(1, FVector(600.0f, 540.0f, 0.0f), 0.0f,
		{
		{FVector(0.0f,50.0f,0.0f), FVector(0.0f) },
		{FVector(50.0f,-50.0f,0.0f), FVector(0.0f)},
		{FVector(-50.0f,-50.0f,0.0f), FVector(0.0f)},
		{FVector(0.0f,50.0f,0.0f), FVector(0.0f)},
		}));
}