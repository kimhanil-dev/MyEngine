#pragma once

#include "IGraphics.h"

#include <vector>
#include <d3dcommon.h>
#include <DirectXMath.h>

using namespace std;
using namespace DirectX;

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11InputLayout;
struct ID3D11Buffer;
struct ID3D11VertexShader;
struct ID3D11PixelShader;

class Graphics : public IGraphics
{
public:
	virtual HRESULT Init(const HWND& hWnd) override;
	virtual void Render() override;
	virtual void Release() override;

private:
	ID3D11Device*			mD3DDevice			= nullptr;
	ID3D11DeviceContext*	mD3DDeviceContext	= nullptr;
	IDXGISwapChain*			mSwapChain			= nullptr;
	ID3D11RenderTargetView*	mRenderTargetView	= nullptr;
	D3D_FEATURE_LEVEL		mFeatureLevel		= D3D_FEATURE_LEVEL_11_0;

	ID3D11InputLayout*	mInputLayout = nullptr;
	ID3D11Buffer*		mVertexBuffer = nullptr;
	ID3D11Buffer*		mIndexBuffer = nullptr;

	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	ID3D11Buffer*		mConstantBuffer = nullptr;

	ID3D11VertexShader* mVertexShader = nullptr;
	ID3D11PixelShader*	mPixelShader = nullptr;

	// Inherited via IGraphics
	void SetCamera(Object* object) override;
	virtual void AddObject(Object* object);
};