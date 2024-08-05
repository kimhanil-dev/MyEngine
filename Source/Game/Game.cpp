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
#include "Utill/PerformanceTester.h"

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
	for (auto& gm : mGeometryMods)
	{
		if (!gm.expired())
		{
			gm.lock()->SetTransform(Matrix::MakeRTMatrix({ 25.0f, rotation ,0.0f }, { 5.0f,0.0f,10.0f }));
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
	mInputManager->BindInput(VK_LEFT, InputManager::KeyState::Down, [this](unsigned int key, InputManager::KeyState state) {mCurrState = mCurrState == State::Pause ? State::Idle : State::Pause; });

	//mGeometryMods.emplace_back(mRenderer->BindMesh(fbx::GetMesh(1)));

	GeometryGenerator geoGen;
	/*Mesh cylinder;
	geoGen.CreateCylinder(5.0f, 5.0f, 5.0f, 5, 5, cylinder);
	auto mod = mRenderer->BindMesh(&cylinder);
	mGeometryMods.push_back(mod);*/
	Mesh sphere;
	geoGen.CreateSphere(5.0f, 20, sphere);
	auto mod = mRenderer->BindMesh(&sphere);
	mGeometryMods.push_back(mod);
}