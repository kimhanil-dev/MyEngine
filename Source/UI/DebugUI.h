#pragma once

#include <Windows.h>
#include <map>
#include <any>

struct ID3D11Device;
struct ID3D11DeviceContext;

class DebugUI
{
public:
	static void Init(const HWND& hWnd, ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext);
	/// <summary>
	/// DirectX11을 통해 UI를 그리는 함수입니다.
	/// DirectX11의 ClearRenderTargetView()와 SwapChain::Present() 사이에 DebugUI::Render()함수가 위치해야 합니다.
	/// </summary>
	static void Render();
	/// <summary>
	/// 프로그램이 종료 되기전에 반드시 호출해야 합니다.
	/// </summary>
	static void Release();

public:
	template <typename T>
	static void SetData(const char* key, const T& data)
	{
		gDatas[key] = data;
	}

	template <typename T>
	static T GetData(const char* key)
	{
		if (gDatas.find(key) != gDatas.end())
		{
			return Cast<T>(gDatas[key]);
		}

		return (T)0;
	}

protected:
	static std::map<const char*, std::any> gDatas;

	template <typename T>
	static T Cast(std::any data)
	{
		try
		{
			return std::any_cast<T>(data);
		}
		catch (const std::bad_any_cast&)
		{
			return (T)0;
		}
	}
};

