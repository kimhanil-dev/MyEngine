#include "pch.h"
#include "Game.h"

#include <DirectXMath.h>

#include "Utill/console.h"
#include "Utill/fbx.h"
#include "Core/Object/Object.h"
#include "Core/Render/Graphics/IGraphics.h"
#include "Core/Render/Graphics/IGeometryModifier.h"
#include "Core/Input/InputManager.h"
#include "Core/Framwork/Timer.h"

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

	fbx::Release();

	mObjects.clear();

	mRenderer->Release();
}

void Game::OnResize()
{
	Application::OnResize();

	if (mRenderer->IsInited())
		mRenderer->ResizeWindow(mClientWidth,mClientHeight);
}

bool Game::Init()
{
	if (!Application::Init())
		return false;

	if (!mRenderer->Init(mhMainWnd))
		return false;

	//
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
	static FMatrix4x4 transMat = Matrix::MakeTranslationMatrix({ 0.0f,0.0f ,50.0f });

	rotation += rotateSpeed * deltaTime;
	for (auto& gm : mGeometryMods)
	{
		rot = Matrix::MakeRotationMatrix({ 90.0f,0.0f, rotation});
		gm->SetTransform(rot * transMat);
	}
}

void Game::LoadGame()
{
	fbx::LoadFBX("./Resource/fbx/box.fbx", 0);
	fbx::LoadFBX("./Resource/fbx/dragon.fbx", 1);

	Object* moveable = new Object(0, FVector(0.0f, 0.0f, 300.0f), 0.0f, fbx::GetMesh(1));

	mInputManager->BindInput(VK_F1, InputManager::KeyState::Down, [this](unsigned int key, InputManager::KeyState state) {mCurrState = State::Destroy, SendMessage(mhMainWnd, WM_CLOSE, NULL, NULL); });
	mInputManager->BindInput(VK_F2, InputManager::KeyState::Down, [this](unsigned int key, InputManager::KeyState state) {mCurrState = mCurrState == State::Pause ? State::Idle : State::Pause; });
	//mInputManager->BindInput(VK_F3, InputManager::KeyState::Down, [this](unsigned int key, InputManager::KeyState state) {mRenderType = mRenderType == Renderer::DriectX ? Renderer::Software : Renderer::DriectX; });
	mInputManager->BindInput(VK_LEFT, InputManager::KeyState::Down, [moveable](unsigned int key, InputManager::KeyState state) {moveable->mOrigin.X -= 10.0f; });
	//mInputManager->BindInput_MousePos([](InputManager::MousePos pos) {Print("Mouse X{%d}:Y{%d}\n", pos.X, pos.Y); });


	IGeometryModifier* modifier = mRenderer->BindMesh(fbx::GetMesh(1));
	mGeometryMods.push_back(modifier);

}