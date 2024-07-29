#include "pch.h"
#include "Application.h"

#include <sstream>
#include <format>

#include "Core/Input/InputManager.h"
#include "Core/Framwork/Timer.h"

// dummy Application instance for avoid nullptr test at WndProc
class DummyApp : public Application
{
public:
	DummyApp(HINSTANCE hAppInst) : Application(hAppInst) {}

	// Inherited via Application
	void OnResize() override {}
	void DrawScene() override {}
	void UpdateScene(float deltaTime) override {}
};


Application::Application(HINSTANCE hInstance)
	: mhAppInst(hInstance),
	mTimer(make_unique<GameTimer>()),
	mInputManager(make_unique<InputManager>())
{
}

Application::~Application()
{
}

HINSTANCE Application::AppInst() const
{
	return mhAppInst;
}

HWND Application::MainWnd() const
{
	return mhMainWnd;
}

float Application::AspectRatio() const
{
	return mClientWidth / (float)mClientHeight;
}

bool Application::Init()
{
	InitMainWindow();

	mTimer->Reset();

	return true;
}

void Application::OnResize()
{
	RECT rc;
	GetClientRect(mhMainWnd, &rc);

	mClientWidth = rc.right;
	mClientHeight = rc.bottom;
}

int Application::Run()
{
	MSG msg = {};

	// WinLoop
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			mTimer->Tick();
			switch (mCurrState)
			{
			case Application::State::Idle:
			{
				CalculateFrameState();
				UpdateScene(mTimer->DeltaTime());
				DrawScene();
			}
			break;
			case Application::State::Pause:
			{
				Sleep(100);
			}
			break;
			}
		}
	}

	return (int)msg.wParam;
}

// imgui forward declaration
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, uint msg, WPARAM wParam, LPARAM lParam);
// WndProc for call FrameWork::WndProc()
static LRESULT CALLBACK _WndProc(HWND hWnd, uint msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	static Application* game = new DummyApp(NULL); // dummy instance for avoid 'if(game != nullptr)'
	game->WndProc(hWnd, msg, wParam, lParam);

	switch (msg)
	{
	case WM_CREATE:
	{
		CREATESTRUCT* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);

		delete game;
		game = reinterpret_cast<Application*>(createStruct->lpCreateParams);
	}
	return 0;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}
LRESULT Application::WndProc(HWND hWnd, uint msg, WPARAM wParam, LPARAM lParam)
{
	if (mInputManager != nullptr)
		static_cast<IWinMsgListener*>(mInputManager.get())->OnListen(msg, wParam, lParam);

	switch (msg)
	{
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			mCurrState = State::Pause;
			mTimer->Stop();
		}
		else
		{
			mCurrState = State::Idle;
			mTimer->Start();
		}
		return 0;
	case WM_ENTERSIZEMOVE:
		mCurrState = State::Pause;
		mTimer->Stop();
		return 0;
	case WM_EXITSIZEMOVE:
		mCurrState = State::Idle;
		mTimer->Start();
		OnResize();
		return 0;
	}

	return 0;
}

bool Application::InitMainWindow()
{
	WNDCLASSEXW wcex = {};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = _WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = mhAppInst;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = L"DirectX11_GameEngine";

	RegisterClassExW(&wcex);

	mhMainWnd = CreateWindowW(wcex.lpszClassName, L"My Engine", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, mhAppInst, this);
	if (mhMainWnd == NULL)
	{
		assert(false);
		return false;
	}

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);

	return true;
}

void Application::CalculateFrameState()
{
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;
	++frameCnt;
	if ((mTimer->TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt;
		float mspf = 1000.0f / fps;
		std::wostringstream outs;
		outs.precision(6);
		outs << std::format(L"{} FPS: {} Frame Time: {} (ms)", L"temp", fps, mspf);
		SetWindowText(mhMainWnd, outs.str().c_str());

		//Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}
