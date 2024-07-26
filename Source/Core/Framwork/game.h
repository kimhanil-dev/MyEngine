#pragma once

#include <vector>
#include <Windows.h>

#include "Core/Render/Graphics/GraphicsFactory.h"
#include "Window/IWinMsgListener.h"
#include "Core/Input/inputManager.h"

#include "Core/Types.h"

using namespace std;

class IGraphics;
class Object;

class FrameWork : public IWinMsgListener
{
public:
	FrameWork() = default;

	void Init(HWND hWnd, HINSTANCE hInstance);
	void Update();
	void Release();

	//IWInMsgListener
	virtual void OnListen(const unsigned int& msg, const __int64& wParam, const __int64& lParam) override;

private:
	HWND mhWnd = NULL;
	HINSTANCE mhInstance = NULL;

	enum class State
	{
		Idle,
		Pause,
		Destroy
	};
	State mCurState = State::Idle;

	IGraphics* mRenderers[(uint)Renderer::Count];
	Renderer mRenderType = Renderer::DriectX;

	Object* mCamera = nullptr;
	vector<Object*> mObjects;

	unique_ptr<InputManager> mInputManager;
};

void UpdateInput(uint key);
