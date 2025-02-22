#include "pch.h"
#include "Graphics.h"

#include <fstream>

#include "Core/Render/Mesh.h"
#include "Utill/console.h"
#include "Utill/frame.h"
#include "Utill/D3DUtill.h"
#include "UI/DebugUI.h"


struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Col;
};

struct ConstantBuffer
{
	XMMATRIX WorldViewPrjection;
};

bool Graphics::Init(const HWND& hWnd)
{
	mhWnd = hWnd;

	HR(BuildDevice());
	HR(BuildFX());
	HR(BuildVertexLayout());
	HR(BuildGeometryBuffers());

	//*** initialize matrices
	mWorld = XMMatrixIdentity();
	mView = XMMatrixLookAtLH({ 0.0f,1.0f,-5.0f,0.0f }, { 0.0f,1.0f,0.0f, 0.0f }, { 0.0f,1.0f,0.0f, 0.0f });
	mProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2, mWndClientWidth / (FLOAT)mWndClientHeight, 0.1f, 100.0f);

	DebugUI::Init(hWnd, mD3DDevice.Get(), mD3DImmediateContext.Get());

	bIsInited = true;

	return true;
}

void Graphics::ResizeWindow(uint width, uint height)
{
	mWndClientWidth = width;
	mWndClientHeight = height;

	mD3DImmediateContext->OMSetRenderTargets(0, NULL, NULL);
	{
		mSwapChain->ResizeBuffers(0, mWndClientWidth, mWndClientHeight, DXGI_FORMAT_UNKNOWN, 0);

		HR(CreateRenderTargetView());
		HR(CreateDepthStencilView());
	}
	mD3DImmediateContext->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), mDepthStencilView.Get());


	// update viewport width, height
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)mWndClientWidth;
	vp.Height = (FLOAT)mWndClientHeight;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	mD3DImmediateContext->RSSetViewports(1, &vp);

	// update view projection matrix's aspect ratio
	mProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2, mWndClientWidth / (FLOAT)mWndClientHeight, 0.1f, 100.0f);
}

void Graphics::Render()
{
	assert(mD3DImmediateContext);
	assert(mSwapChain);

	//*** Rotate cube
	static float rotation = 0.0f;
	mWorld = XMMatrixRotationY(rotation += 0.0001f);

	//*** Clear back buffer
	float clearColor[4] = { 0.0f, 0.125f, 0.6f, 1.0f }; //RGBA
	mD3DImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), clearColor);
	//mD3DImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	mD3DImmediateContext->IASetInputLayout(mInputLayout.Get());
	mD3DImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//*** set geomatry buffers
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	for (const GeometryBuffers& buffers : mGeometries)
	{
		mD3DImmediateContext->IASetVertexBuffers(0, 1, buffers.mVB.GetAddressOf(), &stride, &offset);
		mD3DImmediateContext->IASetIndexBuffer(buffers.mIB.Get(), DXGI_FORMAT_R32_UINT, 0);

		// set pass's constant 
		XMMATRIX worldViewProj = mWorld * mView * mProjection;
		buffers.mWorldViewProj->SetMatrix((float*)&worldViewProj);

		D3DX11_TECHNIQUE_DESC techDesc;
		mFXTech->GetDesc(&techDesc);
		for (UINT iPass = 0; iPass < techDesc.Passes; ++iPass)
		{
			// update constant buffer
			mFXTech->GetPassByIndex(iPass)->Apply(0, mD3DImmediateContext.Get());

			//*** draw
			// draw cube
			mD3DImmediateContext->DrawIndexed(36, 0, 0);
			// draw pyramid
			mD3DImmediateContext->DrawIndexed(18, 36, 8);
		}
	}
	

	//*** imgui draw
	DebugUI::SetData("FPS", GetFPS());
	DebugUI::Render();

	mSwapChain->Present(0, 0);
}

void Graphics::Release()
{
	DebugUI::Release();
}

void Graphics::SetCamera(Object* object)
{
}

void Graphics::AddObject(Object* object)
{
}

bool Graphics::IsInited()
{
	return bIsInited;
}

HRESULT Graphics::BuildDevice()
{
	RECT rc;
	GetClientRect(mhWnd, &rc);
	mWndClientWidth = rc.right - rc.left;
	mWndClientHeight = rc.bottom - rc.top;

	//*** create d3dDevice
	uint createDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#if defined( _DEBUG ) | defined( DEBUG )
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	int* test = nullptr;

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	uint numFeatureLevels = ARRAYSIZE(featureLevels);

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	{
		Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;
		IF_FAILED_RET(CreateDXGIFactory(__uuidof(IDXGIFactory), (LPVOID*)dxgiFactory.GetAddressOf()));

		//*** query adapters
		int64 dxVersion = 0;
		IDXGIOutput* output = nullptr;
		IDXGIAdapter* adapter = nullptr;
		DXGI_OUTPUT_DESC outputDesc;
		DXGI_ADAPTER_DESC adapterDesc;
		vector<Microsoft::WRL::ComPtr<IDXGIAdapter>> adapters;
		for (uint iAdapter = 0; dxgiFactory->EnumAdapters(iAdapter, &adapter) != DXGI_ERROR_NOT_FOUND; ++iAdapter)
		{
			adapters.emplace_back(adapter);

			adapters[iAdapter]->GetDesc(&adapterDesc);

			adapters[iAdapter]->CheckInterfaceSupport(__uuidof(IDXGIDevice), (LARGE_INTEGER*)&dxVersion);
			Print(L"Detect Adapter{%d}: [Device Driver Version: %d], [Desc: %s]\n",
				adapterDesc.DeviceId, dxVersion, adapterDesc.Description);
			for (uint iOutput = 0; adapters[iAdapter]->EnumOutputs(iOutput, &output) != DXGI_ERROR_NOT_FOUND; ++iOutput)
			{
				output->GetDesc(&outputDesc);
				Print(L"\tConnected Output: {%s}\n", outputDesc.DeviceName);

				uint modeNum = 0;
				output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, NULL, &modeNum, NULL);
				DXGI_MODE_DESC* displayModes = new DXGI_MODE_DESC[modeNum];
				output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, NULL, &modeNum, displayModes);
				for (uint iMode = 0; iMode < modeNum; ++iMode)
				{
					Print("\t\tMode: WIDTH = %d HEIGHT = %d REFRESH = %d/%d\n", 
						displayModes[iMode].Width, displayModes[iMode].Height, displayModes[iMode].RefreshRate.Numerator, displayModes[iMode].RefreshRate.Denominator);
				}
				delete[] displayModes;
			}
		}
	}

	//
	IF_FAILED_RET(D3D11CreateDevice(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		createDeviceFlags,
		NULL, 0,
		D3D11_SDK_VERSION,
		mD3DDevice.GetAddressOf(),
		&featureLevel,
		mD3DImmediateContext.GetAddressOf()));


	//*** create swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = mWndClientWidth;
	sd.BufferDesc.Height = mWndClientHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = mhWnd;
	if (mbEnable4xMsaa)
	{
		sd.SampleDesc.Count = 4;
		m4xMsaaQuality = CheckMultisampleQualityLevels(sd.BufferDesc.Format, sd.SampleDesc.Count);
		sd.SampleDesc.Quality = m4xMsaaQuality - 1;
	}
	else
	{
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Windowed = true;

	Microsoft::WRL::ComPtr<IDXGIDevice>		dxgiDevice;
	Microsoft::WRL::ComPtr<IDXGIAdapter>	dxgiAdapter;
	Microsoft::WRL::ComPtr<IDXGIFactory>	dxgiFactory;

	IF_FAILED_RET(mD3DDevice->QueryInterface(__uuidof(IDXGIDevice), (LPVOID*)dxgiDevice.GetAddressOf()));
	IF_FAILED_RET(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (LPVOID*)dxgiAdapter.GetAddressOf()));
	IF_FAILED_RET(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (LPVOID*)dxgiFactory.GetAddressOf()));

	IF_FAILED_RET(dxgiFactory->CreateSwapChain(mD3DDevice.Get(), &sd, mSwapChain.GetAddressOf()));

	//*** Set DXGI does not monitor the message queue
	dxgiFactory->MakeWindowAssociation(mhWnd, DXGI_MWA_NO_WINDOW_CHANGES);

	//*** create render target and depth stenil
	IF_FAILED_RET(CreateRenderTargetView());
	IF_FAILED_RET(CreateDepthStencilView());

	//*** set render target and depth stencil to om
	mD3DImmediateContext->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), mDepthStencilView.Get());

	//*** set view port
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)mWndClientWidth;
	vp.Height = (FLOAT)mWndClientHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	mD3DImmediateContext->RSSetViewports(1, &vp);

	return S_OK;
}

HRESULT Graphics::BuildGeometryBuffers(const Mesh* inMesh, GeometryBuffers& outGeomtryBuffers)
{
	assert(inMesh);
	assert(inMesh->IndexCount == 0 || inMesh->VertexCount == 0);

	// create vertex buffer
	SimpleVertex* vertices = new SimpleVertex[inMesh->VertexCount];
	{
		FVector position;
		FVector color;
		for (UINT iVertex = 0; iVertex < inMesh->VertexCount; ++iVertex)
		{
			position = inMesh->Vertices[iVertex].Position;
			color = inMesh->Vertices[iVertex].Color;
			vertices[iVertex].Pos = XMFLOAT3(position.X, position.Y, position.Z);
			vertices[iVertex].Col = XMFLOAT4(color.X,color.Y,color.Z,1.0f);
		}

		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(vertices);
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = vertices;

		IF_FAILED_RET(mD3DDevice->CreateBuffer(&bd, &initData, outGeomtryBuffers.mVB.GetAddressOf()));
	}
	delete[] vertices;

	// create index buffer
	assert(sizeof(uint) == sizeof(UINT));
	UINT indexCount = inMesh->IndexCount;
	UINT* indices = (UINT*)inMesh->Indices;

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(UINT) * indexCount;// 36;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initIndexData;
	ZeroMemory(&initIndexData, sizeof(initIndexData));
	initIndexData.pSysMem = inMesh->Indices;

	IF_FAILED_RET(mD3DDevice->CreateBuffer(&indexBufferDesc, &initIndexData, outGeomtryBuffers.mIB.GetAddressOf()));

	return S_OK;
}

HRESULT Graphics::BuildVertexLayout()
{
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	uint numElements = ARRAYSIZE(layout);

	D3DX11_PASS_DESC passDesc;
	mFXTech->GetPassByIndex(0)->GetDesc(&passDesc);
	IF_FAILED_RET(mD3DDevice->CreateInputLayout(layout, numElements, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, mInputLayout.GetAddressOf()));

	return S_OK;
}

HRESULT Graphics::BuildFX(GeometryBuffers& outGeomtryBuffers)
{
	std::ifstream fin("./Shader/color.fxo", std::ios::binary);
	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);
	fin.read(&compiledShader[0], size);
	fin.close();

	IF_FAILED_RET(D3DX11CreateEffectFromMemory((LPVOID)&compiledShader[0],
		compiledShader.size(), NULL, mD3DDevice.Get(), outGeomtryBuffers.mFX.GetAddressOf()));

	outGeomtryBuffers.mWorldViewProj = outGeomtryBuffers.mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	outGeomtryBuffers.mTech = outGeomtryBuffers.mFX->GetTechniqueByName("ColorTech");

	return S_OK;
}

HRESULT Graphics::CreateRenderTargetView()
{
	assert(mD3DDevice != nullptr);
	assert(mSwapChain != nullptr);

	HRESULT hr = S_OK;

	if (mRenderTargetView)
	{
		mRenderTargetView = nullptr;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	IF_FAILED_RET(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)backBuffer.GetAddressOf()));

	IF_FAILED_RET(mD3DDevice->CreateRenderTargetView(backBuffer.Get(), NULL, mRenderTargetView.GetAddressOf()));

	return hr;
}

uint Graphics::CheckMultisampleQualityLevels(const DXGI_FORMAT format, const uint sampleCount)
{
	assert(mD3DDevice != nullptr);

	uint result = 0;
	IF_FAILED_RET(mD3DDevice->CheckMultisampleQualityLevels(format, sampleCount, &result));

	return result;
}

void Graphics::BindMesh(Mesh* mesh)
{
	assert(mesh);

}

HRESULT Graphics::CreateDepthStencilView()
{
	assert(mD3DImmediateContext);
	assert(mD3DDevice);
	assert(mSwapChain);

	HRESULT hr = S_OK;

	if (mDepthStencilView)
	{
		mDepthStencilTexture = nullptr;
		mDepthStencilView = nullptr;
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	mSwapChain->GetDesc(&swapChainDesc);

	D3D11_TEXTURE2D_DESC depthStencilDesc = {};
	
	depthStencilDesc.Width		= mWndClientWidth;
	depthStencilDesc.Height		= mWndClientHeight;
	depthStencilDesc.MipLevels	= 1;
	depthStencilDesc.ArraySize	= 1;
	depthStencilDesc.Format		= DXGI_FORMAT_D24_UNORM_S8_UINT;

	depthStencilDesc.SampleDesc = swapChainDesc.SampleDesc;

	depthStencilDesc.Usage			= D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags		= D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags		= 0;

	HR(mD3DDevice->CreateTexture2D(&depthStencilDesc, 0, mDepthStencilTexture.GetAddressOf()));
	HR(mD3DDevice->CreateDepthStencilView(mDepthStencilTexture.Get(), 0, NULL));

	return hr;
}
