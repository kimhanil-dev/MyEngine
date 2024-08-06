#include "pch.h"
#include "Game.h"

#include "Utill/console.h"
#include "Utill/fbx.h"
#include "Core/Object/Object.h"
#include "Core/Render/Graphics/IGraphics.h"
#include "Core/Input/InputManager.h"
#include "Core/Framwork/Timer.h"
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
	constexpr float rotateSpeed = 1.0f;

	static XMFLOAT3 geoShperePos = { 0.0f,-0.0f, 10.0f };
	static XMFLOAT3 viewDir;
	static XMFLOAT4X4 translationMat; 
	static XMFLOAT3 lightColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
	static float rotation = 0.0f;

	XMVECTOR view = XMLoadFloat3(&viewDir);
	view = XMVector3Normalize(view);
	XMStoreFloat3(&viewDir, view);

	XMMATRIX tm = XMMatrixRotationRollPitchYaw(0.0f, rotation, 0.0f)
		* XMMatrixTranslation(geoShperePos.x, geoShperePos.y, geoShperePos.z);

	XMStoreFloat4x4(&translationMat, tm);

	rotation += rotateSpeed * deltaTime;
	for (auto& gm : mGeometryMods)
	{
		if (!gm.expired())
		{
			gm.lock()->SetFloat3("gLightColor", (float*)&lightColor);
			gm.lock()->SetFloat3("gViewDir", (float*)&viewDir);
			gm.lock()->SetMatrix("gWorld", translationMat);
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
	
	/*Mesh grid;
	geoGen.CreateGrid(50.0f, 50.0f, 30, 30, grid);
	auto mod0 = mRenderer->BindMesh(&grid);
	mGeometryMods.push_back(mod0);*/

	Mesh geoSphere;
	geoGen.CreateGeosphere({0.5f,1.0f,1.0f,1.0f}, 5.0f, 3, geoSphere);
	auto mod = mRenderer->BindMesh(&geoSphere);
	mGeometryMods.push_back(mod);

	/*Mesh cylinder;
	geoGen.CreateCylinder(2.0f, 3.0f, 5.0f, 8, 8, cylinder);
	auto mod1 = mRenderer->BindMesh(&cylinder);
	mGeometryMods.push_back(mod1);*/
}