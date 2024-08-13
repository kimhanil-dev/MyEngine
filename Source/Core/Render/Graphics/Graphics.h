#pragma once

#include "IGraphics.h"

#include <vector>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11effect.h>
#include <wrl.h>

#include "Core/Object/Object.h"
#include "IGeometryModifier.h"

#include "Material.h"


using namespace std;
using namespace DirectX;

struct GeometryBuffers : public IGeometryDynamicModifier
{
	GeometryBuffers() {}
	virtual ~GeometryBuffers() {}

	Microsoft::WRL::ComPtr<ID3DX11Effect> mFX;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mVB;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mIB;
	
	ID3DX11EffectTechnique*			mTech = nullptr;

	static Microsoft::WRL::ComPtr<ID3D11InputLayout> mIL;


	virtual void SetRaw(const char* name, const void* data, const size_t byteSize) override
	{
		assert(mFX);

		if (auto var = mFX->GetVariableByName(name))
		{
			var->SetRawValue(data, 0, byteSize);
		}
	}

	virtual void SetFloat(const char* name, const float& value) override
	{
		assert(mFX);

		if (auto var = mFX->GetVariableByName(name))
		{
			var->AsScalar()->SetFloat(value);
		}
	}
	
	virtual void SetFloat3(const char* name, const float* value) override
	{
		assert(mFX);

		if (auto var = mFX->GetVariableByName(name))
		{
			var->AsVector()->SetFloatVector(value);
		}
	}

	virtual void SetMatrix(const char* name, const XMFLOAT4X4& value) override
	{
		assert(mFX);

		if (auto var = mFX->GetVariableByName(name))
		{
			var->AsMatrix()->SetMatrix((float*)value.m);
		}
	}

	function<void(D3D11_MAPPED_SUBRESOURCE data)> mMapAction = nullptr;
	virtual void BindResourceMap(function<void(D3D11_MAPPED_SUBRESOURCE data)> mapAction) override
	{
		_ASSERT(mapAction);
		_ASSERT(!mMapAction);

		if (mMapAction == nullptr)
			mMapAction = mapAction;
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
	HRESULT BuildGeometryBuffers(const Mesh* inMesh, GeometryBuffers& outGeomtryBuffers, bool isDynamic = false);
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
	weak_ptr<IGeometryDynamicModifier> BindMeshDynamic(Mesh* mesh) override;

	const IObject* mCameraObject		= nullptr;
	const IObject* mPointLightObject	= nullptr;
	const IObject* mSpotLightObject		= nullptr;

	// lights
	DirectionLight mDirLight;
	PointLight mPointLight;
	SpotLight mSpotLight;

	// IGraphics을(를) 통해 상속됨
	void BindPointLight(const IObject* object, PointLight& desc) override;
	void UnBindPointLight(const IObject* lightObject) override;

	void BindSpotLight(const IObject* object, SpotLight& desc) override;
	void UnBindSpotLight(const IObject* lightObject) override;

	// IGraphics을(를) 통해 상속됨
	void BindCameraObject(const IObject* cameraObject) override;
	void UnBindCameraObject(const IObject* cameraObject) override;
};