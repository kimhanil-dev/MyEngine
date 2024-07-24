#pragma once

#include "IGraphics.h"

#include <wrl.h>
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>

using namespace std;
using namespace DirectX;

class Graphics : public IGraphics
{
public:
	virtual HRESULT Init(const HWND& hWnd) override;
	virtual void Render() override;
	virtual void Release() override;

private:
	Microsoft::WRL::ComPtr<ID3D11Device>			mD3DDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>		mD3DDeviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain>			mSwapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	mRenderTargetView;

	Microsoft::WRL::ComPtr<ID3D11InputLayout>	mInputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		mVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		mIndexBuffer;
	
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		mConstantBuffer;

	Microsoft::WRL::ComPtr<ID3D11VertexShader>  mVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	mPixelShader;

	// Inherited via IGraphics
	void SetCamera(Object* object) override;
	virtual void AddObject(Object* object);
};