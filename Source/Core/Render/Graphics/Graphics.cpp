#include "Graphics.h"

#include <cassert>
#include <d3dcompiler.h>

#include "Utill/console.h"
#include "Utill/frame.h"

#include "UI/DebugUI.h"

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
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	//*** create d3dDevice and swapchain
	UINT createDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#if defined( _DEBUG ) | defined( DEBUG )
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
		D3D11_SDK_VERSION, &sd, mSwapChain.GetAddressOf(), mD3DDevice.GetAddressOf(), &featureLevel, mD3DDeviceContext.GetAddressOf());
	if (FAILED(hr))
		return hr;

	//*** set render target
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer = NULL;
	hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)backBuffer.GetAddressOf());
	if (FAILED(hr))
		return hr;

	hr = mD3DDevice->CreateRenderTargetView(backBuffer.Get(), NULL, mRenderTargetView.GetAddressOf());
	if (FAILED(hr))
		return hr;

	mD3DDeviceContext->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), NULL);


	//*** set view port
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
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
	UINT numElements = ARRAYSIZE(layout);

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

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
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
	mProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2, width / (FLOAT)height, 0.1f, 100.0f);

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
