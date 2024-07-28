#pragma once

#include <vector>
#include <Windows.h>

#include "Core/Types.h"

using namespace std;

class InputManager;
class GameTimer;

class Application
{
public:
	Application(HINSTANCE hInstance);
	virtual ~Application();

	HINSTANCE	AppInst() const;
	HWND		MainWnd() const;
	float		AspectRatio() const;
	int			Run();

	virtual bool Init();
	virtual void OnResize();
	virtual void DrawScene() = 0;
	virtual LRESULT CALLBACK WndProc(HWND hWnd, uint msg, WPARAM wParam, LPARAM lParam);

protected:
	virtual void UpdateScene(float deltaTime) = 0;
	
	bool InitMainWindow();
	void CalculateFrameState();
	//virtual void LoadResources();

protected:
	HWND mhMainWnd		= NULL;
	HINSTANCE mhAppInst = NULL;

	uint mClientWidth	= 800;
	uint mClientHeight	= 640;

	enum class State
	{
		Idle,
		Pause,
		Destroy
	};
	State mCurrState = State::Idle;

	unique_ptr<GameTimer> mTimer;
	unique_ptr<InputManager> mInputManager;
};

