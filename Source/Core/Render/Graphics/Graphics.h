#pragma once

#include "IGraphics.h"

#include <vector>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11effect.h>
#include <DirectXMath.h>
#include <wrl.h>

#include "Core/Math/Matrix.h"
#include "IGeometryModifier.h"


using namespace std;
using namespace DirectX;

struct GeometryBuffers : public IGeometryModifier
{
	GeometryBuffers() : mWorld(XMMatrixIdentity()){}
	virtual ~GeometryBuffers() {}

	Microsoft::WRL::ComPtr<ID3DX11Effect> mFX;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mVB;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mIB;
	
	XMMATRIX						mWorld;
	ID3DX11EffectMatrixVariable*	mWorldViewProj = nullptr;
	ID3DX11EffectTechnique*			mTech = nullptr;

	static Microsoft::WRL::ComPtr<ID3D11InputLayout> mIL;

	virtual void SetFloat(const char* name, const float& value)
	{
		assert(mFX);

		if (auto var = mFX->GetVariableByName(name))
		{
			var->AsScalar()->SetFloat(value);
		}
	}
	virtual void SetTransform(const FMatrix4x4& worldTransMat) override
	{
		mWorld = XMMATRIX(worldTransMat.m);
	}
};

class Graphics : public IGraphics
{
public:
	virtual bool Init(const HWND& hWnd) override;
	virtual void Render() override;
	virtual void Release() override;
private:
	HRESULT BuildDevice();
	HRESULT BuildGeometryBuffers(const Mesh* inMesh, GeometryBuffers& outGeomtryBuffers);
	HRESULT BuildVertexLayout(GeometryBuffers& geometryBuffers);
	HRESULT BuildFX(GeometryBuffers& outGeomtryBuffers);
private:
	std::vector<shared_ptr<GeometryBuffers>> mGeometries;

	HWND mhWnd = NULL;
	unsigned int mWndClientWidth = 0;
	unsigned int mWndClientHeight = 0;

	bool mbEnable4xMsaa = true;
	uint m4xMsaaQuality = 0;

	Microsoft::WRL::ComPtr<ID3D11Device>			mD3DDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>		mD3DImmediateContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain>			mSwapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	mRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	mDepthStencilView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>			mDepthStencilTexture;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> mDepthStencilState;
	ID3DX11EffectScalarVariable* mTime;

	XMMATRIX mView;
	XMMATRIX mProjection;

	// Inherited via IGraphics
	void SetCamera(Object* object) override;
	virtual void AddObject(Object* object) override;

	bool bIsInited = false;
	virtual bool IsInited() override;

	void ResizeWindow(uint width, uint height) override;

	/// <summary>
	/// clear and recreate RenderTargetView and DepthStencilView and apply them
	/// </summary>
	HRESULT CreateRenderTargetView();
	HRESULT CreateDepthStencilView();
	uint CheckMultisampleQualityLevels(const DXGI_FORMAT format, const uint sampleCount);

	// IGraphics을(를) 통해 상속됨
	weak_ptr<IGeometryModifier> BindMesh(Mesh* mesh) override;
};