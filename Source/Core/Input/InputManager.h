#pragma once

#include <functional>

#include "Window/IWinMsgListener.h"

using namespace std;

/// <summary>
/// <para> - Input에 대한 기능을 정의하기 위한 클래스로 Microsoft에서 정의한 'VK_*' 입력 범위를 지원합니다.</para>
/// <para> - Input 상태에 대하여 1개의 InputEvent만 바인딩 가능합니다.</para>
/// <para> - TODO : Callback을 등록한 인스턴스가 메모리에서 삭제되었을때의 처리를 추가해야 합니다.</para>
/// </summary>
class InputManager : public IWinMsgListener
{
//----- Button
public:
	enum class KeyState : unsigned int
	{
		Idle,	// 바인딩에서 사용하지 않습니다. (내부 전용)

		Down,
		Click,	// Down 상태에서 입력키가 때졌을때의 상태입니다.
		Press,
		Up,		// Press 상태에서 입력키가 때졌을때의 상태입니다.

		SIZE,	// 바인딩에서 사용하지 않습니다. KeyState의 길이를 측정하기 위한 값 입니다.
	};

	typedef function<void(unsigned int, KeyState)> InputEvent;

	/// <summary>
	///		input에 대한 액션을 정의하기 위해, 함수를 등록하며
	///		조합키는 작동하지 않습니다.
	/// </summary>
	/// <param name="key">
	///		바인딩할 Keyboard input ID (VK_*)
	/// </param>
	/// <param name="callback">
	///		`key`에 대한 이벤트를 수신할 함수
	/// </param>
	/// <returns>
	///		`key`에 대한 바인딩이 이미 존재할 경우 false 반환
	/// </returns>
	void BindInput(unsigned int key, KeyState keyState, InputEvent callback);
	/// <summary>
	///		BindKey()함수를 통해 등록된 기능을 해제합니다.
	/// </summary>
	/// <param name="key">
	///		바인딩된 Keyboard input ID (VK_*)
	/// </param>
	void UnBindInput(unsigned int key, KeyState keyState);

protected:
	static constexpr unsigned int KEY_RANGE = VK_OEM_CLEAR + 1;

	// 메모리 절약을 위해 바인딩에서 사용하지 않는
	// KeyState::Idle을 제거한 만큼 배열을 선언합니다.
	// KeyState::Idle이 제거되거나, binding에서 사용되지 않는 키 입력이 발생할 경우
	// GetInputEventRef()의 내용을 수정하여야 합니다.
	InputEvent	mInputBindings[KEY_RANGE][(unsigned int)KeyState::SIZE - 1] = {};
	KeyState	mKeyStates[KEY_RANGE] = {KeyState::Idle};

	/// <summary>
	/// key에 바인딩된 InputEvent에 대한 일관적인 접근을 제공하기 위한 함수입니다.
	/// </summary>
	/// <param name="key"></param>
	/// <param name="keyState"></param>
	/// <returns></returns>
	InputEvent& GetInputEventRef(const unsigned int& key, const KeyState& keyState);

//----- Mouse
public:
	struct MousePos
	{
		unsigned int X = 0;
		unsigned int Y = 0;
	};

	typedef function<void(const MousePos& mousePox)> MouseMoveEvent;
	/// <summary>
	/// Mouse의 Position이 업데이트 될때마다 호출되는 함수를 등록합니다.
	/// </summary>
	/// <param name="callback"></param>
	void BindInput_MousePos(MouseMoveEvent callback);

protected:
	MousePos mMousePos;
	vector<MouseMoveEvent> mMouseMoveBindings;

//----- Window Message Listener
protected:
	// IWinMsgListener을(를) 통해 상속됨
	void OnListen(const unsigned int& msg, const __int64& wParam, const __int64& lParam) override;
};

