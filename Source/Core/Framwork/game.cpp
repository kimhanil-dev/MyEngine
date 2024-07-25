#include "pch.h"
#include "game.h"

#include "Utill/console.h"
#include "Utill/fbx.h"
#include "Core/Object/Object.h"
#include "Core/Render/Graphics/IGraphics.h"

void FrameWork::Init(HWND hWnd, HINSTANCE hInstance)
{
	mhWnd = hWnd;

	HRESULT hr = S_OK;

	// Init renderers
	mRenderers[(UINT)Renderer::DriectX] = GetRenderer(Renderer::DriectX);
	mRenderers[(UINT)Renderer::Software] = GetRenderer(Renderer::Software);

	for (auto& renderer : mRenderers)
	{
		if (FAILED(hr = renderer->Init(mhWnd)))
		{
			PrintError("Renderer init failed : %x\n", hr);
			assert(false);
		}
	}

	fbx::LoadFBX("./Resource/fbx/Box.fbx", 0);
	fbx::LoadFBX("./Resource/fbx/dragon.fbx", 1);

	Object* moveable = new Object(0, FVector(0.0f, 0.0f, 300.0f), 0.0f, fbx::GetMesh(1));

	// init input manager
	if (mInputManager == nullptr)
		mInputManager = make_unique<InputManager>();
	mInputManager->BindInput(VK_F1, InputManager::KeyState::Down, [this](unsigned int key, InputManager::KeyState state) {mCurState = State::Destroy, SendMessage(mhWnd, WM_CLOSE, NULL, NULL); });
	mInputManager->BindInput(VK_F2, InputManager::KeyState::Down, [this](unsigned int key, InputManager::KeyState state) {mCurState = mCurState == State::Pause ? State::Idle : State::Pause; });
	mInputManager->BindInput(VK_F3, InputManager::KeyState::Down, [this](unsigned int key, InputManager::KeyState state) {mRenderType = mRenderType == Renderer::DriectX ? Renderer::Software : Renderer::DriectX; });
	mInputManager->BindInput(VK_LEFT, InputManager::KeyState::Down, [moveable](unsigned int key, InputManager::KeyState state) {moveable->mOrigin.X -= 10.0f; });
	mInputManager->BindInput_MousePos([](InputManager::MousePos pos) {Print("Mouse X{%d}:Y{%d}\n", pos.X, pos.Y); });

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
}

void FrameWork::Update()
{
	static ULONGLONG prevTime = GetTickCount64();
	ULONGLONG time = GetTickCount64();
	ULONGLONG deltaTime = time - prevTime;
	float deltaSec = deltaTime / 1000.0f;

	static float refreshTime = 0.0f;
	refreshTime += deltaSec;

	mRenderers[(UINT)mRenderType]->Render();

	switch (mCurState)
	{
	case FrameWork::State::Idle:
	{
		constexpr float rotateSpeed = 50.0f;
		for (int i = 0; i < mObjects.size(); ++i)
		{
			mObjects[i]->mRotate += rotateSpeed * deltaSec;
		}
	}
	break;
	case FrameWork::State::Pause:
		break;
	default:
		break;
	}

	prevTime = time;
}

void FrameWork::Release()
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

void FrameWork::OnListen(const unsigned int& msg, const __int64& wParam, const __int64& lParam)
{
	if (mInputManager != nullptr)
		static_cast<IWinMsgListener*>(mInputManager.get())->OnListen(msg, wParam, lParam);

	if (msg == WM_SIZE)
	{
		//TODO : fix
		if(mRenderers[(UINT)mRenderType] != nullptr)
			mRenderers[(UINT)mRenderType]->ResizeWindow(LOWORD(lParam), HIWORD(lParam));
	}
}
