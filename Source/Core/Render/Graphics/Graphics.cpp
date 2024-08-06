#include "pch.h"
#include "Graphics.h"

#include <fstream>

#include "Core/Render/Mesh.h"
#include "Utill/console.h"
#include "Utill/D3DUtill.h"
#include "UI/DebugUI.h"

Microsoft::WRL::ComPtr<ID3D11InputLayout> GeometryBuffers::mIL = nullptr;

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Tangent;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex0;
	XMFLOAT2 Tex1;
	XMFLOAT4 Color;
};

struct ConstantBuffer
{
	XMMATRIX WorldViewPrjection;
};

bool Graphics::Init(const HWND& hWnd)
{
	mhWnd = hWnd;

	HR(BuildDevice());

	// move to BindMesh();
		//HR(BuildFX());
		//HR(BuildVertexLayout());
		//HR(BuildGeometryBuffers());

	//*** initialize matrices
	mView = XMMatrixLookAtLH({ 0.0f,1.0f,-5.0f,0.0f }, { 0.0f,1.0f,0.0f, 0.0f }, { 0.0f,1.0f,0.0f, 0.0f });
	mProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2, mWndClientWidth / (FLOAT)mWndClientHeight, 0.1f, 1000.0f);

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
	D3D11_VIEWPORT vp{};
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


	//*** update resources
	for (auto geos : mGeometries)
	{
		if (geos->mMapAction != nullptr)
		{
			D3D11_MAPPED_SUBRESOURCE mappedData;
			HR(mD3DImmediateContext->Map(geos->mVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

			geos->mMapAction(mappedData);

			mD3DImmediateContext->Unmap(geos->mVB.Get(), 0);
		}
	}

	//*** Clear back buffer
	float clearColor[4] = { 0.0f, 0.125f, 0.6f, 1.0f }; //RGBA
	mD3DImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), clearColor);
	mD3DImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	if (GeometryBuffers::mIL == nullptr)
		return;

	mD3DImmediateContext->IASetInputLayout(GeometryBuffers::mIL.Get());
	mD3DImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//*** set geomatry buffers
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	GeometryBuffers* buffers = nullptr;
	size_t size = mGeometries.size();
	for (int i = 0; i < size; ++i)
	{
		buffers = mGeometries[i].get();

		mD3DImmediateContext->IASetVertexBuffers(0, 1, buffers->mVB.GetAddressOf(), &stride, &offset);
		mD3DImmediateContext->IASetIndexBuffer(buffers->mIB.Get(), DXGI_FORMAT_R32_UINT, 0);

		// set pass's constant 
		XMMATRIX vp = mView * mProjection;
		buffers->mFX->GetVariableByName("gViewProj")->AsMatrix()->SetMatrix((float*)&vp);

		D3DX11_TECHNIQUE_DESC techDesc;
		buffers->mTech->GetDesc(&techDesc);
		for (UINT iPass = 0; iPass < techDesc.Passes; ++iPass)
		{
			// update constant buffer
			buffers->mTech->GetPassByIndex(iPass)->Apply(0, mD3DImmediateContext.Get());

			D3D11_BUFFER_DESC bd;
			buffers->mIB->GetDesc(&bd);
			mD3DImmediateContext->DrawIndexed(bd.ByteWidth / sizeof(UINT), 0, 0);
		}
	}

	mSwapChain->Present(0, 0);
}

void Graphics::Release()
{
	DebugUI::Release();
}

void Graphics::SetView(const XMFLOAT4X4& viewMatrix)
{
	XMStoreFloat4x4(const_cast<XMFLOAT4X4*>(&viewMatrix), mView);
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

HRESULT Graphics::BuildGeometryBuffers(const Mesh* inMesh, GeometryBuffers& outGeomtryBuffers, bool isDynamic)
{
	assert(inMesh);

	// create vertex buffer
	UINT vertexCount = static_cast<UINT>(inMesh->Vertices.size());

	vector<SimpleVertex> vertices;
	vertices.resize(vertexCount);

	const vector<Vertex>& meshVertices = inMesh->Vertices;
	UINT index = 0;
	for (const Vertex& v : meshVertices)
	{
		vertices[index].Pos = v.Position;
		// tangent
		vertices[index].Normal = v.Normal;
		vertices[index].Tex0 = v.UV;
		// tex1
		vertices[index].Color = v.Color;

		++index;
	}

	D3D11_BUFFER_DESC bd{};
	bd.Usage = isDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(SimpleVertex) * vertexCount;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = isDynamic ? D3D10_CPU_ACCESS_WRITE : 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = vertices.data();

	IF_FAILED_RET(mD3DDevice->CreateBuffer(&bd, &initData, outGeomtryBuffers.mVB.GetAddressOf()));

	// create index buffer
	const vector<UINT>& indices = inMesh->Indices;

	D3D11_BUFFER_DESC indexBufferDesc{};
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(UINT) * static_cast<UINT>(indices.size());// 36;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initIndexData{};
	initIndexData.pSysMem = indices.data();

	IF_FAILED_RET(mD3DDevice->CreateBuffer(&indexBufferDesc, &initIndexData, outGeomtryBuffers.mIB.GetAddressOf()));

	return S_OK;
}

HRESULT Graphics::BuildVertexLayout(GeometryBuffers& geometryBuffers)
{
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 52, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	uint numElements = ARRAYSIZE(layout);

	D3DX11_PASS_DESC passDesc;
	geometryBuffers.mTech->GetPassByIndex(0)->GetDesc(&passDesc);
	IF_FAILED_RET(mD3DDevice->CreateInputLayout(layout, numElements, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, GeometryBuffers::mIL.GetAddressOf()));

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

weak_ptr<IGeometryModifier> Graphics::BindMesh(Mesh* mesh)
{
	assert(mesh);

	auto gbs = mGeometries.emplace_back(new GeometryBuffers);
	BuildFX(*gbs);
	BuildGeometryBuffers(mesh, *gbs);

	if (!GeometryBuffers::mIL)
	{
		HR(BuildVertexLayout(*gbs));
	}

	return gbs;
}

weak_ptr<IGeometryDynamicModifier> Graphics::BindMeshDynamic(Mesh* mesh)
{
	assert(mesh);

	auto gbs = mGeometries.emplace_back(new GeometryBuffers);
	BuildFX(*gbs);
	BuildGeometryBuffers(mesh, *gbs, true);

	if (!GeometryBuffers::mIL)
	{
		HR(BuildVertexLayout(*gbs));
	}

	return gbs;
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

	HR(mD3DDevice->CreateTexture2D(&depthStencilDesc, 0, mDepthStencilTexture.GetAddressOf()));
	HR(mD3DDevice->CreateDepthStencilView(mDepthStencilTexture.Get(), 0, mDepthStencilView.GetAddressOf()));

	return hr;
}
