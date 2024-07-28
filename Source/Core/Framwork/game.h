#pragma once

#include <vector>
#include <Windows.h>

#include "Core/Render/Graphics/GraphicsFactory.h"
#include "Window/IWinMsgListener.h"
#include "Core/Input/inputManager.h"

#include "Core/Types.h"

using namespace std;

class GameTimer;
class IGraphics;
class Object;

class FrameWork : public IWinMsgListener
{
public:
	FrameWork() = default;

	void Init(HWND hWnd, HINSTANCE hInstance);
	int Run();
	void DrawScene();
	void Release();

	//IWInMsgListener
	virtual void OnListen(const unsigned int& msg, const __int64& wParam, const __int64& lParam) override;
private:
	void UpdateScene(float deltaTime);
	void LoadResources();

private:
	HWND mhWnd = NULL;
	HINSTANCE mhInstance = NULL;

	enum class State
	{
		Idle,
		Pause,
		Destroy
	};
	State mCurrState = State::Idle;

	IGraphics* mRenderers[(uint)Renderer::Count];
	Renderer mRenderType = Renderer::DriectX;

	shared_ptr<GameTimer> mTimer;
	Object* mCamera = nullptr;
	vector<Object*> mObjects;

	
	unique_ptr<InputManager> mInputManager;
};

