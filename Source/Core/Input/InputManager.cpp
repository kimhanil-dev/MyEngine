#include "pch.h"
#include "InputManager.h"

#include <windowsx.h>

#include "Utill/console.h"

InputManager::InputManager()
{
}

void InputManager::BindInput(unsigned int key, KeyState keyState, InputEvent callback)
{
	// out of range
	assert(key < KEY_RANGE);
	assert(keyState != KeyState::Idle && L"Idle은 허용하지 않습니다.");
	assert(keyState != KeyState::SIZE && L"유효하지 않은 값 입니다.");

	InputEvent& inputEvent = GetInputEventRef(key, keyState);
	{
		assert(!inputEvent); // key is already binded
		inputEvent = callback;
	}
}

void InputManager::UnBindInput(unsigned int key, KeyState keyState)
{
	if (InputEvent& inputEvent = GetInputEventRef(key,keyState))
		inputEvent = InputEvent();
}

void InputManager::Update()
{
	for (UINT i = 0; i < KEY_RANGE; ++i)
	{
		if (mKeyStates[i] == KeyState::Press)
		{
			if (InputEvent& inputEvent = GetInputEventRef(i, KeyState::Press))
			{
				inputEvent(i, KeyState::Press);
			}
		}
	}
}

void InputManager::BindInput_MousePos(MouseMoveEvent callback)
{
	mMouseMoveBindings.emplace_back(callback);
}

void InputManager::OnListen(const unsigned int& msg, const __int64& wParam, const __int64& lParam)
{
	// mouse input processing
	if (msg == WM_MOUSEMOVE)
	{
		mMousePos.X = GET_X_LPARAM(lParam);
		mMousePos.Y = GET_Y_LPARAM(lParam);
		
		for (auto& callback : mMouseMoveBindings)
		{
			callback(mMousePos);
		}
		return;
	}

	// down, up input processing
	if (msg != WM_KEYDOWN && msg != WM_KEYUP)
		return;

	if (wParam >= KEY_RANGE)
		return;

	unsigned int key = static_cast<unsigned int>(wParam);
	KeyState& keyState = mKeyStates[key];

	if (msg == WM_KEYDOWN)
	{
		switch (keyState)
		{
		case InputManager::KeyState::Idle:
			keyState = KeyState::Down;
			break;
		case InputManager::KeyState::Down:
			keyState = KeyState::Press;
			break;
		default:
			return;
		}
	}
	else if (msg == WM_KEYUP)
	{
		switch (keyState)
		{
		case InputManager::KeyState::Down:
			keyState = KeyState::Click;
			break;
		case InputManager::KeyState::Press:
			keyState = KeyState::Up;
			break;
		default:
			return;
		}
	}

	Print("Key {%d}, State {%d}\n", key, keyState);

	if (InputEvent& inputEvent = GetInputEventRef(key, keyState))
		inputEvent(key, keyState);

	if (keyState == KeyState::Click || keyState == KeyState::Up)
		keyState = KeyState::Idle;
}

InputManager::InputEvent& InputManager::GetInputEventRef(const unsigned int& key, const KeyState& keyState)
{
	// 메모리 공간을 절약하기 위해 Idle을 제거한 KeyState enum의 길이를 사용하기 때문에
	// keyState의 index에서 1만큼 감소시켜 접근합니다.
	return mInputBindings[key][(unsigned int)keyState - 1];
}
