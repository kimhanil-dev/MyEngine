#include "pch.h"
#include "Game.h"

#include <DirectXMath.h>

#include "Utill/console.h"
#include "Utill/fbx.h"
#include "Core/Object/Object.h"
#include "Core/Render/Graphics/IGraphics.h"
#include "Core/Input/InputManager.h"
#include "Core/Framwork/Timer.h"
#include "Core/Render/Mesh.h"

#include "Core/Render/GeometryGenerator.h"

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
	constexpr float rotateSpeed = 50.0f;
	static float rotation = 0.0f;
	static FMatrix4x4 rot{};
	static FMatrix4x4 transMat = Matrix::MakeTranslationMatrix({ 0.0f,0.0f ,100.0f });

	rotation += rotateSpeed * deltaTime;
	for (auto gm : mGeometryMods)
	{
		if (gm.expired())
		{
			gm.lock()->SetFloat("gTime", mTimer->TotalTime());
		}
	}
}

void Game::LoadGame()
{
	fbx::LoadFBX("./Resource/fbx/box.fbx", 0);
	fbx::LoadFBX("./Resource/fbx/dragon.fbx", 1);

	mInputManager->BindInput(VK_F1, InputManager::KeyState::Down, [this](unsigned int key, InputManager::KeyState state) {mCurrState = State::Destroy, SendMessage(mhMainWnd, WM_CLOSE, NULL, NULL); });
	mInputManager->BindInput(VK_F2, InputManager::KeyState::Down, [this](unsigned int key, InputManager::KeyState state) {mCurrState = mCurrState == State::Pause ? State::Idle : State::Pause; });

	mGeometryMods.emplace_back(mRenderer->BindMesh(fbx::GetMesh(1)));

	GeometryGenerator geoGen;
	Mesh gridMesh;
	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, gridMesh);
	auto mod = mRenderer->BindMesh(&gridMesh);
	if (mod.expired())
	{
		mod.lock()->SetTransform(Matrix::MakeTranslationMatrix({ 0.0f,-50.0f,150.0f }));
		mGeometryMods.push_back(mod);
	}

}