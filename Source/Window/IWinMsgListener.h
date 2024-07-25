#pragma once

class IWinMsgListener
{
public:
	/// <summary>
	///  Window 메세지를 수신합니다.
	/// </summary>
	/// <param name="msg">
	///		윈도우 메시지 타입
	/// </param>
	/// <param name="wParam">
	///		메시지에 연관된 데이터1
	/// </param>
	/// <param name="lParam">
	///		메시지에 연관된 데이터2
	/// </param>
	virtual void OnListen(const unsigned int& msg, const __int64& wParam, const __int64& lParam) = 0;
};