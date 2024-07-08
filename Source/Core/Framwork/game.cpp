#include "game.h"

#include "Core/Render/Render.h"
#include "Core/Object/Object.h"
#include "Utill/console.h"
#include "Utill/fbx.h"

#include <iostream>
#include <functional>
#include <vector>
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
static constexpr float FPS_60_DELTA_TIME = 1.0f / 60.0f;
static constexpr float FPS_144_DELTA_TIME = 1.0f / 144.0f;

// input
	// event
typedef std::function<void(UINT_PTR)> InputEvent;
std::map<UINT_PTR, InputEvent> gInputEventListeners;

void Init(HWND hWnd, const uint width, const uint height)
{
	ghWnd = hWnd;

	Render::Init(hWnd,width,height);

	// add input event listener
	gInputEventListeners[VK_F1] = [](UINT_PTR key) {gbIsGameRunning = false, SendMessage(ghWnd, WM_CLOSE, NULL, NULL); };	// redraw screen
	gInputEventListeners[VK_F2] = [](UINT_PTR key) {gbIsPause = gbIsPause == false ? true : false; };	// redraw screen

	fbx::LoadFBX("D:/CGPractice/KWorld/Resource/fbx/Box.fbx", 0);
	fbx::LoadFBX("D:/CGPractice/KWorld/Resource/fbx/dragon.fbx", 1);

	gObjects.push_back(new Object(0, FVector(0.0f, 0.0f, 300.0f),0.0f,fbx::GetMesh(0)));

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



	// camera

	PrintGood("Init complite");
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
	if (refreshTime >= FPS_144_DELTA_TIME)
	{
		refreshTime -= FPS_144_DELTA_TIME;
		Render::Render(&camera, gObjects);
	}

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

	Render::Release();
}
