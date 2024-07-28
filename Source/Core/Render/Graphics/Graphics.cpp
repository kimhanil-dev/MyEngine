#include "pch.h"
#include "Graphics.h"

#include "Utill/console.h"
#include "Utill/frame.h"

#include "UI/DebugUI.h"

#define IF_FAILED_RETURN(func) hr = func; if(FAILED(hr)) return hr;
#define IF_FAILED_LOG_AND_RETURN(hr) if(FAILED(hr)) {PrintError("hr failed : code {%x}", hr); return hr;} 

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Col;
};

struct ConstantBuffer
{
	XMMATRIX World;
	XMMATRIX View;
	XMMATRIX Projection;
};

HRESULT Graphics::Init(const HWND& hWnd)
{
	mhWnd = hWnd;

	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(hWnd, &rc);
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

	//
	hr = D3D11CreateDevice(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		createDeviceFlags,
		NULL, 0,
		D3D11_SDK_VERSION,
		mD3DDevice.GetAddressOf(),
		&featureLevel,
		mD3DDeviceContext.GetAddressOf());
	if(FAILED(hr))
	{
		MessageBox(mhWnd, L"D3D11CreateDevice failed", 0, 0);
		return  hr;
	}

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
	sd.OutputWindow = hWnd;
	if (mbEnable4xMsaa)
	{
		sd.SampleDesc.Count = 4;
		m4xMsaaQuality =  CheckMultisampleQualityLevels(sd.BufferDesc.Format, sd.SampleDesc.Count);
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

	IF_FAILED_RETURN(mD3DDevice->QueryInterface(__uuidof(IDXGIDevice),	(LPVOID*)dxgiDevice.GetAddressOf()));
	IF_FAILED_RETURN(dxgiDevice->GetParent(__uuidof(IDXGIAdapter),		(LPVOID*)dxgiAdapter.GetAddressOf()));
	IF_FAILED_RETURN(dxgiAdapter->GetParent(__uuidof(IDXGIFactory),		(LPVOID*)dxgiFactory.GetAddressOf()));

	IF_FAILED_RETURN(dxgiFactory->CreateSwapChain(mD3DDevice.Get(), &sd, mSwapChain.GetAddressOf()));

	//*** create render target and depth stenil
	IF_FAILED_RETURN(CreateRenderTargetView());
	IF_FAILED_RETURN(CreateDepthStencilView());

	//*** set render target and depth stencil to om
	mD3DDeviceContext->OMSetRenderTargets(1,mRenderTargetView.GetAddressOf(), mDepthStencilView.Get());

	//*** set view port
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)mWndClientWidth;
	vp.Height = (FLOAT)mWndClientHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	mD3DDeviceContext->RSSetViewports(1, &vp);

	//*** compile shader
	Microsoft::WRL::ComPtr<ID3DBlob> errorMsg = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = nullptr;
	hr = D3DCompileFromFile(L"Shader/SimpleVS.fx", NULL, NULL, "VS", "vs_4_0",
		D3DCOMPILE_ENABLE_STRICTNESS, NULL, vsBlob.GetAddressOf(), errorMsg.GetAddressOf());
	if (FAILED(hr))
	{
		if (errorMsg != nullptr)
		{
			PrintError("Shader compile failed : %s\n", (char*)errorMsg->GetBufferPointer());
		}

		return hr;
	}

	hr = mD3DDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, mVertexShader.GetAddressOf());
	if (FAILED(hr))
	{
		return hr;
	}


	//*** create input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	uint numElements = ARRAYSIZE(layout);

	hr = mD3DDevice->CreateInputLayout(layout, numElements, vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(), mInputLayout.GetAddressOf());
	if (FAILED(hr))
		return hr;

	mD3DDeviceContext->IASetInputLayout(mInputLayout.Get());

	Microsoft::WRL::ComPtr<ID3DBlob> psBlob = nullptr;
	hr = D3DCompileFromFile(L"Shader/SimplePS.fx", NULL, NULL, "PS", "ps_4_0",
		D3DCOMPILE_ENABLE_STRICTNESS, NULL, psBlob.GetAddressOf(), errorMsg.GetAddressOf());
	if (FAILED(hr))
	{
		if (errorMsg != nullptr)
		{
			PrintError("Shader compile failed : %s\n", (char*)errorMsg->GetBufferPointer());
		}
		return hr;
	}

	hr = mD3DDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, mPixelShader.GetAddressOf());
	if (FAILED(hr))
	{
		return hr;
	}

	//*** create vertex buffer
	SimpleVertex vertices[] =
	{
		{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
	};


	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 8;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = vertices;

	hr = mD3DDevice->CreateBuffer(&bd, &initData, mVertexBuffer.GetAddressOf());
	if (FAILED(hr))
		return hr;

	uint stride = sizeof(SimpleVertex);
	uint offset = 0;
	mD3DDeviceContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);

	//*** Create index buffer
	WORD indices[] =
	{
		3,1,0,
		2,1,3,

		0,5,4,
		1,5,0,

		3,4,7,
		0,4,3,

		1,6,5,
		2,6,1,

		2,7,6,
		3,7,2,

		6,4,5,
		7,4,6,
	};

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(WORD) * 36;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initIndexData;
	ZeroMemory(&initIndexData, sizeof(initIndexData));
	initIndexData.pSysMem = indices;

	hr = mD3DDevice->CreateBuffer(&indexBufferDesc, &initIndexData, mIndexBuffer.GetAddressOf());
	if (FAILED(hr))
		return hr;

	// Set index buffer
	mD3DDeviceContext->IASetIndexBuffer(mIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	// set primitive topology
	mD3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Create constant buffer
	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = 0;
	hr = mD3DDevice->CreateBuffer(&constantBufferDesc, NULL, mConstantBuffer.GetAddressOf());
	if (FAILED(hr))
		return hr;

	//*** initialize matrices
	mWorld = XMMatrixIdentity();
	mView = XMMatrixLookAtLH({ 0.0f,1.0f,-5.0f,0.0f }, { 0.0f,1.0f,0.0f, 0.0f }, { 0.0f,1.0f,0.0f, 0.0f });
	mProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2, mWndClientWidth / (FLOAT)mWndClientHeight, 0.1f, 100.0f);

	DebugUI::Init(hWnd, mD3DDevice.Get(), mD3DDeviceContext.Get());

	return S_OK;
}

void Graphics::Render()
{
	//*** Rotate cube
	//mWorld = XMMatrixRotationY();

	//*** Clear back buffer
	float clearColor[4] = { 0.0f, 0.125f, 0.6f, 1.0f }; //RGBA
	mD3DDeviceContext->ClearRenderTargetView(mRenderTargetView.Get(), clearColor);

	//*** Update variables
	ConstantBuffer cb;
	cb.World = XMMatrixTranspose(mWorld);
	cb.View = XMMatrixTranspose(mView);
	cb.Projection = XMMatrixTranspose(mProjection);
	mD3DDeviceContext->UpdateSubresource(mConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

	//*** Render
	mD3DDeviceContext->VSSetShader(mVertexShader.Get(), NULL, 0);
	mD3DDeviceContext->VSSetConstantBuffers(0, 1, mConstantBuffer.GetAddressOf());
	mD3DDeviceContext->PSSetShader(mPixelShader.Get(), NULL, 0);
	mD3DDeviceContext->DrawIndexed(36, 0, 0);

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

void Graphics::ResizeWindow(uint width, uint height)
{
	mWndClientWidth = width;
	mWndClientHeight = height;

	HRESULT hr = S_OK;

	/*
	// Alt + Enter 단축키 or IDXGISwapChain::SetFullScreenState()를 통해 전체 화면으로 변경될때
	// 이 경우 DXGI는 Front Buffer를 최대 해상도 [ex) 1920x1080]으로 설정한다.
	// 그러나 WM_SIZE 메세지를 통해 전달되는 해상도는 메뉴바의 높이를 제외한(1080 - MenuBarHeight) 크기를 전달 받게 된다.
	// 따라서, 이 값으로 재 설정한 BackBuffer는 Front Buffer와 해상도 차이(Menu Bar 높이 만큼)가 발생하게 된다.
	// 이를 해결하기 위해, GetFullScreenState() == true 일때 메인 모니터의 전체화면 해상도를 불러와 Backbuffer를 초기화 시킨다.
	BOOL bIsFullScreen = false;
	IDXGIOutput* mainOutput = nullptr;

	mSwapChain->GetContainingOutput(&mainOutput);
	mSwapChain->GetFullscreenState(&bIsFullScreen, &mainOutput);

	if (bIsFullScreen)
	{
		IDXGIOutput1* output1;
		hr = mainOutput->QueryInterface(&output1);
		if (FAILED(hr))
			assert(false);

		DXGI_SWAP_CHAIN_DESC dc;
		mSwapChain->GetDesc(&dc);

		uint num = 0;
		output1->GetDisplayModeList1(dc.BufferDesc.Format, NULL, &num, NULL);

		if (num != 0)
		{
			DXGI_MODE_DESC1* descs = new DXGI_MODE_DESC1[num];
			output1->GetDisplayModeList1(dc.BufferDesc.Format, NULL, &num, descs);

			for (int i = 0; i < num; ++i)
			{
				if (descs[i].Width >= mWndClientWidth && descs[i].Height >= mWndClientHeight)
				{
					mWndClientWidth = descs[i].Width;
					mWndClientHeight = descs[i].Height;
				}
			}

			delete[] descs;
		}

		output1->Release();
		mainOutput->Release();
	}
	*/

	mD3DDeviceContext->OMSetRenderTargets(0, NULL, NULL);
	{
		mSwapChain->ResizeBuffers(0, mWndClientWidth, mWndClientHeight, DXGI_FORMAT_UNKNOWN, 0);

		hr = CreateRenderTargetView();
		if (FAILED(hr))
		{
			MessageBox(mhWnd, L"CreateRenderTargetView failed", 0, 0);
			return;
		}

		hr = CreateDepthStencilView();
		if (FAILED(hr))
		{
			MessageBox(mhWnd, L"CreateDepthStencilView failed", 0, 0);
			return;
		}
	}
	mD3DDeviceContext->OMSetRenderTargets(1,mRenderTargetView.GetAddressOf(), mDepthStencilView.Get());


	// update viewport width, height
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)mWndClientWidth;
	vp.Height = (FLOAT)mWndClientHeight;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	mD3DDeviceContext->RSSetViewports(1, &vp);

	// update view projection matrix's aspect ratio
	mProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2, mWndClientWidth / (FLOAT)mWndClientHeight, 0.1f, 100.0f);
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
	hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)backBuffer.GetAddressOf());
	if(FAILED(hr))
		return hr;

	hr = mD3DDevice->CreateRenderTargetView(backBuffer.Get(), NULL, mRenderTargetView.GetAddressOf());
	if(FAILED(hr))
		return hr;

	return hr;
}

uint Graphics::CheckMultisampleQualityLevels(const DXGI_FORMAT format, const uint sampleCount)
{
	assert(mD3DDevice != nullptr);

	uint result = 0;
	mD3DDevice->CheckMultisampleQualityLevels(format, sampleCount, &result);

	return result;
}

HRESULT Graphics::CreateDepthStencilView()
{
	assert(mD3DDevice != nullptr);
	assert(mSwapChain != nullptr);

	HRESULT hr = S_OK;

	if (mDepthStencilView)
	{
		mDepthStencilTexture = nullptr;
		mDepthStencilView = nullptr;
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	mSwapChain->GetDesc(&swapChainDesc);

	D3D11_TEXTURE2D_DESC depthStencilDesc = {};
	depthStencilDesc.Width = mWndClientWidth;
	depthStencilDesc.Height = mWndClientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc = swapChainDesc.SampleDesc;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	hr = mD3DDevice->CreateTexture2D(&depthStencilDesc, 0, mDepthStencilTexture.GetAddressOf());
	IF_FAILED_LOG_AND_RETURN(hr);
	hr = mD3DDevice->CreateDepthStencilView(mDepthStencilTexture.Get(), 0, mDepthStencilView.GetAddressOf());
	IF_FAILED_LOG_AND_RETURN(hr);

	return hr;
}
