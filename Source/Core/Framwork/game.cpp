#include "game.h"

#include "Core/Object/Object.h"
#include "Utill/console.h"
#include "Utill/fbx.h"
#include "Core/Render/Graphics/IGraphics.h"


#include <iostream>
#include <functional>
#include <vector>
#include <cassert>
#include <map>

// window
	// handle
HWND ghWnd = NULL;

// game
	// state
bool gbIsGameRunning = true;
bool gbIsPause = false;
// object
static std::vector<Object*> gObjects;
// camera
static Object camera;

// frame
static float constexpr FPS_60_DELTA_TIME = 1.0f / 60.0f;
static float constexpr FPS_144_DELTA_TIME = 1.0f / 144.0f;

//render
IGraphics* gRenderer[] = {
	GetRenderer(Renderer::DriectX),
	GetRenderer(Renderer::Software),
};

// input
	// event
typedef std::function<void(UINT_PTR)> InputEvent;
std::map<UINT_PTR, InputEvent> gInputEventListeners;

// 0 : SW
// 1 : D2D
Renderer gRenderType = Renderer::Software;

void Init(HWND hWnd, const uint width, const uint height)
{
	ghWnd = hWnd;

	// Init Renderer
	HRESULT hr = S_OK;

	// Init renderers
	for (auto& renderer : gRenderer)
	{
		if (FAILED(hr = renderer->Init(ghWnd)))
		{
			PrintError("Renderer init failed : %x\n", hr);
			assert(false);
		}
	}


	fbx::LoadFBX("./Resource/fbx/Box.fbx", 0);
	fbx::LoadFBX("./Resource/fbx/dragon.fbx", 1);

	Object* moveable = new Object(0, FVector(0.0f, 0.0f, 300.0f), 0.0f, fbx::GetMesh(1));

	// add input event listener
	gInputEventListeners[VK_F1] = [](UINT_PTR key) {gbIsGameRunning = false, SendMessage(ghWnd, WM_CLOSE, NULL, NULL); };	// redraw screen
	gInputEventListeners[VK_F2] = [](UINT_PTR key) {gbIsPause = gbIsPause == false ? true : false; };	// redraw screen
	gInputEventListeners[VK_F3] = [](UINT_PTR key) {gRenderType = gRenderType == Renderer::DriectX ? Renderer::Software : Renderer::DriectX; };		// change render type
	gInputEventListeners[VK_LEFT] = [moveable](UINT_PTR key) {moveable->mOrigin.X -= 10.0f; };		// change render type


	gObjects.push_back(moveable);

	// triangle
	gObjects.push_back(new Object(1, FVector(500.0f, 540.0f, 0.0f), 0.0f,
		{
		{FVector(0.0f,-50.0f,0.0f), FVector(1.0f,0.0f,0.0f)},
		{FVector(50.0f,50.0f,0.0f), FVector(0.0f,1.0f,0.0f) },
		{FVector(-50.0f,50.0f,0.0f), FVector(0.0f,0.0f,1.0f)},
		{FVector(0.0f,-50.0f,0.0f), FVector(0.0f)},
		}));

	gObjects.push_back(new Object(1, FVector(600.0f, 540.0f, 0.0f), 0.0f,
		{
		{FVector(0.0f,50.0f,0.0f), FVector(0.0f) },
		{FVector(50.0f,-50.0f,0.0f), FVector(0.0f)},
		{FVector(-50.0f,-50.0f,0.0f), FVector(0.0f)},
		{FVector(0.0f,50.0f,0.0f), FVector(0.0f)},
		}));

	for (auto object : gObjects)
	{
		for (auto& renderer : gRenderer)
		{
			renderer->AddObject(object);
		}
	}

	// camera
	for (auto& renderer : gRenderer)
	{
		renderer->SetCamera(&camera);
	}

	//PrintGood("Init complite");
}

bool Update()
{
	static ULONGLONG prevTime = GetTickCount64();
	ULONGLONG time = GetTickCount64();
	ULONGLONG deltaTime = time - prevTime;
	float deltaSec = deltaTime / 1000.0f;

	static float refreshTime = 0.0f;
	refreshTime += deltaSec;

	// 60fps
	//if (refreshTime >= FPS_144_DELTA_TIME)
	//{
	//	refreshTime -= FPS_144_DELTA_TIME;
	//}
	gRenderer[static_cast<unsigned int>(gRenderType)]->Render();

	if (!gbIsPause)
	{
		constexpr float rotateSpeed = 50.0f;
		for (int i = 0; i < gObjects.size(); ++i)
		{
			gObjects[i]->mRotate += rotateSpeed * deltaSec;
		}
	}

	prevTime = time;

	return gbIsGameRunning;
}

void UpdateInput(uint key)
{
	auto inputListener = gInputEventListeners.find(key);
	if (inputListener != gInputEventListeners.end())
	{
		inputListener->second(key);
	}
}

void Release()
{

	for (int i = 0; i < gObjects.size(); ++i)
	{
		delete gObjects[i];
	}

	fbx::Release();

	gObjects.clear();

	for (auto& renderer : gRenderer)
		renderer->Release();
}
