#include "DirectX11Wrapper.h"

HRESULT DirectX11Wrapper::Create(HWND hwnd, RECT rc)
{
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes = 0;
	size_t stringLength;
	DXGI_ADAPTER_DESC adapterDesc;

	// グラフィックインタフェースファクトリを作成
	HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(hr)) return hr;

	int GPUNumber = 0;
	int GPUMaxMem = 0;

	// GPUアダプタを検索
	for (int i = 0; i < 100; i++)
	{
		IDXGIAdapter* add;
		if (FAILED(factory->EnumAdapters(i, &add)))
			break;
		hr = add->GetDesc(&adapterDesc);

		char videoCardDescription[128];
		// ビデオカード名を取得
		int error = wcstombs_s(&stringLength, videoCardDescription, 128, adapterDesc.Description, 128);
		if (error != 0)
		{
			break;
		}

		// ビデオカードメモリを取得（MB単位）
		int videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

		// アウトプット（モニター）に番号IDを付ける
		if (FAILED(add->EnumOutputs(0, &adapterOutput)))
		{
			continue;
		}

		// DXGI_FORMAT_R8G8B8A8_UNORM の表示形式数を取得する
		if (FAILED(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL)))
		{
			continue;
		}

		if (videoCardMemory > GPUMaxMem)
		{
			GPUMaxMem = videoCardMemory;
			GPUNumber = i;
		}
		add->Release();

		// アダプタアウトプットを解放
		adapterOutput->Release();
		adapterOutput = 0;
	}

	// グラフィックインタフェースアダプターを作成
	if (FAILED(factory->EnumAdapters(GPUNumber, &adapter)))
	{
		return E_FAIL;
	}

#pragma region DirectX11Init
	UINT cdev_flags = 0;
#ifdef _DEBUG
	cdev_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// スワップチェイン設定
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = rc.right;
	sd.BufferDesc.Height = rc.bottom;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hwnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
	};

	// DirectX11デバイスとスワップチェイン作成
	if (FAILED(D3D11CreateDeviceAndSwapChain(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, cdev_flags,
		featureLevels, 1, D3D11_SDK_VERSION, &sd, &m_SwapChain, &m_Device, NULL, &m_ImmediateContext)))
	{
		return E_FAIL;
	}

	// アダプタを解放
	adapter->Release();
	adapter = 0;

	// ファクトリを解放
	factory->Release();
	factory = 0;

	ComPtr<ID3D11Texture2D> pBackBuffer;
	// スワップチェインに用意されたバッファ（2Dテクスチャ）を取得
	if (FAILED(m_SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)))) {
		return E_FAIL;
	}

	// レンダーターゲットView作成
	if (FAILED(m_Device->CreateRenderTargetView(pBackBuffer.Get(), NULL, &m_RenderTargetView))) {
		return hr;
	}

	// 深度ステンシルバッファ作成
	D3D11_TEXTURE2D_DESC txDesc;
	ZeroMemory(&txDesc, sizeof(txDesc));
	txDesc.Width = rc.right;
	txDesc.Height = rc.bottom;
	txDesc.MipLevels = 1;
	txDesc.ArraySize = 1;
	txDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	txDesc.SampleDesc.Count = 1;
	txDesc.SampleDesc.Quality = 0;
	txDesc.Usage = D3D11_USAGE_DEFAULT;
	txDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	txDesc.CPUAccessFlags = 0;
	txDesc.MiscFlags = 0;

	if (FAILED(m_Device->CreateTexture2D(&txDesc, NULL, m_DepthStencilTexture.ReleaseAndGetAddressOf())))
	{
		return E_FAIL;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));
	dsDesc.Format = txDesc.Format;
	dsDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsDesc.Texture2D.MipSlice = 0;

	if (FAILED(m_Device->CreateDepthStencilView(m_DepthStencilTexture.Get(), &dsDesc, m_DepthStencilView.ReleaseAndGetAddressOf())))
	{
		return E_FAIL;
	}

	// ビューポート設定
	m_ViewPort.Width = static_cast<FLOAT>(rc.right - rc.left);
	m_ViewPort.Height = static_cast<FLOAT>(rc.bottom - rc.top);
	m_ViewPort.MinDepth = 0.0f;
	m_ViewPort.MaxDepth = 1.0f;
	m_ViewPort.TopLeftX = 0;
	m_ViewPort.TopLeftY = 0;
#pragma endregion

	CreateTexture();

	return S_OK;
}


void DirectX11Wrapper::Release()
{
}

// ポリゴンの初期化
bool DirectX11Wrapper::PolygonInit()
{
	Vertex VertexList[]{
		{ { -0.5f,  0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f }, {0.0f,0.0f}},
		{ {  0.5f, -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f }, {1.0f,0.0f}},
		{ { -0.5f, -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f }, {0.0f,1.0f}},
		{ {  0.5f,  0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f }, {1.0f,1.0f}}
	};

	//頂点バッファ作成
	D3D11_BUFFER_DESC VBDesc;
	VBDesc.ByteWidth = sizeof(Vertex) * 4;
	VBDesc.Usage = D3D11_USAGE_DEFAULT;
	VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBDesc.CPUAccessFlags = 0;
	VBDesc.MiscFlags = 0;
	VBDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA VBResouceData;
	VBResouceData.pSysMem = VertexList;
	VBResouceData.SysMemPitch = 0;
	VBResouceData.SysMemSlicePitch = 0;

	// 頂点バッファ作成
	if (FAILED(m_Device->CreateBuffer(&VBDesc, &VBResouceData, m_VertexBuffer.ReleaseAndGetAddressOf())))
	{
		return false;
	}

	WORD IndexList[]{
		0, 1, 2,
		0, 3, 1,
	};

	IndexNum = _countof(IndexList);

	D3D11_BUFFER_DESC IBDesc;
	IBDesc.ByteWidth = sizeof(WORD) * 6;
	IBDesc.Usage = D3D11_USAGE_DEFAULT;
	IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IBDesc.CPUAccessFlags = 0;
	IBDesc.MiscFlags = 0;
	IBDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA IBResouceData;
	IBResouceData.pSysMem = IndexList;
	IBResouceData.SysMemPitch = 0;
	IBResouceData.SysMemSlicePitch = 0;

	if (FAILED(m_Device->CreateBuffer(&IBDesc, &IBResouceData, m_IndexBuffer.ReleaseAndGetAddressOf())))
	{
		return false;
	}

	// 頂点シェーダコンパイル
	ComPtr<ID3DBlob> vsblob;
	CompileShader("Shader/BasicVS.hlsl", "main", "vs_5_0", vsblob.ReleaseAndGetAddressOf());

	// 頂点シェーダー作成
	if (FAILED(m_Device->CreateVertexShader(vsblob->GetBufferPointer(), vsblob->GetBufferSize(), NULL, m_VertexShader.ReleaseAndGetAddressOf())))
	{
		return E_FAIL;
	}

	// 頂点データ設定
	D3D11_INPUT_ELEMENT_DESC VertexDesc[]
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT   ,0,0                           ,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"COLOR"   ,0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"NORMAL" , 0,DXGI_FORMAT_R32G32B32_FLOAT,   0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,		 0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
	};

	// インプットレイアウト作成
	if (FAILED(m_Device->CreateInputLayout(VertexDesc, ARRAYSIZE(VertexDesc), vsblob->GetBufferPointer(), vsblob->GetBufferSize(), m_InputLayOut.ReleaseAndGetAddressOf())))
	{
		return E_FAIL;
	}

	// ピクセルシェーダーコンパイル
	ComPtr<ID3DBlob> psblob;
	CompileShader("Shader/TexturePS.hlsl", "main", "ps_5_0", psblob.ReleaseAndGetAddressOf());

	// ピクセルシェーダー作成
	if (FAILED(m_Device->CreatePixelShader(psblob->GetBufferPointer(), psblob->GetBufferSize(), NULL, m_PixelShader.ReleaseAndGetAddressOf())))
	{
		return E_FAIL;
	}

	//定数バッファ設定
	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(ConstantBuffer);
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	// 定数バッファ作成
	if (FAILED(m_Device->CreateBuffer(&cbDesc, NULL, m_ConstantBuffer.ReleaseAndGetAddressOf())))
	{
		return false;
	}

	// ワールド座標変換
	XMMATRIX worldMatrix = XMMatrixTranslation(0.0f, 0.0f, 0.0f);

	// ビュー座標
	XMVECTOR eye = XMVectorSet(0.0f, 0.0f, -2.0f, 0.0f);		// 視点位置
	XMVECTOR focus = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);		// 注視点
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);			// 上ベクトル
	XMMATRIX viewMatrix = XMMatrixLookAtLH(eye, focus, up);		// ビュー変換

	// プロジェクション変換
	float    fov = XMConvertToRadians(45.0f);					// 視野角
	float    aspect = m_ViewPort.Width / m_ViewPort.Height;		// アスペクト比
	float    nearZ = 0.1f;										// 近
	float    farZ = 100.0f;										// 遠
	XMMATRIX projMatrix = XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);

	// 各変換行列セット
	ConstantBuffer cb;
	XMStoreFloat4x4(&cb.world, XMMatrixTranspose(worldMatrix));
	XMStoreFloat4x4(&cb.view, XMMatrixTranspose(viewMatrix));
	XMStoreFloat4x4(&cb.projection, XMMatrixTranspose(projMatrix));

	// ライト
	XMVECTOR Light = XMVector3Normalize(XMVectorSet(0.0f, 0.5f, -1.0f, 0.0f));
	XMStoreFloat4(&cb.LightDir, Light);

	m_ImmediateContext->UpdateSubresource(m_ConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

	return true;
}


void DirectX11Wrapper::ObjectDraw()
{
	UINT strides = sizeof(Vertex);
	UINT offsets = 0;
	m_ImmediateContext->IASetInputLayout(m_InputLayOut.Get());
	m_ImmediateContext->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &strides, &offsets);
	m_ImmediateContext->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	m_ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_ImmediateContext->VSSetConstantBuffers(0, 1, m_ConstantBuffer.GetAddressOf());
	m_ImmediateContext->VSSetShader(m_VertexShader.Get(), NULL, 0);
	m_ImmediateContext->PSSetShader(m_PixelShader.Get(), NULL, 0);

	m_ImmediateContext->PSSetShaderResources(0, 1, NoiseTextureResouce.GetAddressOf());
	m_ImmediateContext->PSSetSamplers(0, 1, m_SamplerState.GetAddressOf());

	m_ImmediateContext->DrawIndexed(IndexNum, 0, 0);
}

bool DirectX11Wrapper::CubeInit()
{
	Vertex VertexList[]{
	{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f } ,{  0.0f,  0.0f, -1.0f }, {0.0f,0.0f}},
	{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f } ,{  0.0f,  0.0f, -1.0f }, {1.0f,0.0f}},
	{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f } ,{  0.0f,  0.0f, -1.0f }, {0.0f,1.0f}},
	{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f } ,{  0.0f,  0.0f, -1.0f }, {1.0f,1.0f}},

	{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f }, {0.0f,0.0f}},
	{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f }, {1.0f,0.0f}},
	{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f }, {0.0f,1.0f}},
	{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f }, {1.0f,1.0f}},

	{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f, 0.0f, 1.0f }, { -1.0f,  0.0f,  0.0f }, {0.0f,0.0f}},
	{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f, 0.0f, 1.0f }, { -1.0f,  0.0f,  0.0f }, {1.0f,0.0f}},
	{ { -0.5f, -0.5f,  0.5f }, { 1.0f, 1.0f, 0.0f, 1.0f }, { -1.0f,  0.0f,  0.0f }, {0.0f,1.0f}},
	{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f, 0.0f, 1.0f }, { -1.0f,  0.0f,  0.0f }, {1.0f,1.0f}},

	{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f }, {  1.0f,  0.0f,  0.0f } , {0.0f,0.0f}},
	{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f }, {  1.0f,  0.0f,  0.0f } , {1.0f,0.0f}},
	{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f }, {  1.0f,  0.0f,  0.0f } , {0.0f,1.0f}},
	{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f }, {  1.0f,  0.0f,  0.0f } , {1.0f,1.0f}},

	{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 1.0f, 1.0f }, {  0.0f,  1.0f,  0.0f }  , {0.0f,0.0f}},
	{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 1.0f, 1.0f }, {  0.0f,  1.0f,  0.0f }  , {1.0f,0.0f}},
	{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 1.0f, 1.0f }, {  0.0f,  1.0f,  0.0f }  , {0.0f,1.0f}},
	{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 1.0f, 1.0f }, {  0.0f,  1.0f,  0.0f }  , {1.0f,1.0f}},

	{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f }, {  0.0f, -1.0f,  0.0f } , {0.0f,0.0f}},
	{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f }, {  0.0f, -1.0f,  0.0f } , {1.0f,0.0f}},
	{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f }, {  0.0f, -1.0f,  0.0f } , {0.0f,1.0f}},
	{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f }, {  0.0f, -1.0f,  0.0f } , {1.0f,1.0f}},
	};

	WORD IndexList[]{
		 0,  1,  2,     3,  2,  1,
		 4,  5,  6,     7,  6,  5,
		 8,  9, 10,    11, 10,  9,
		12, 13, 14,    15, 14, 13,
		16, 17, 18,    19, 18, 17,
		20, 21, 22,    23, 22, 21,
	};

	IndexNum = ARRAYSIZE(IndexList);

	//頂点バッファ作成
	D3D11_BUFFER_DESC vbDesc;
	vbDesc.ByteWidth = sizeof(Vertex) * ARRAYSIZE(VertexList);
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;
	vbDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vrData;
	vrData.pSysMem = VertexList;
	vrData.SysMemPitch = 0;
	vrData.SysMemSlicePitch = 0;

	if (FAILED(m_Device->CreateBuffer(&vbDesc, &vrData, m_VertexBuffer.ReleaseAndGetAddressOf())))
	{
		return false;
	}


	//インデックスバッファ作成
	D3D11_BUFFER_DESC ibDesc;
	ibDesc.ByteWidth = sizeof(WORD) * ARRAYSIZE(IndexList);
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.MiscFlags = 0;
	ibDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA irData;
	irData.pSysMem = IndexList;
	irData.SysMemPitch = 0;
	irData.SysMemSlicePitch = 0;

	if (FAILED(m_Device->CreateBuffer(&ibDesc, &irData, m_IndexBuffer.ReleaseAndGetAddressOf())))
	{
		return false;
	}

	ComPtr<ID3DBlob> pErrorBlob = NULL;

	// 頂点シェーダコンパイル
	ComPtr<ID3DBlob> vsblob;
	CompileShader("Shader/BasicVS.hlsl", "main", "vs_5_0", vsblob.ReleaseAndGetAddressOf());

	// 頂点シェーダー作成
	if (FAILED(m_Device->CreateVertexShader(vsblob->GetBufferPointer(), vsblob->GetBufferSize(), NULL, m_VertexShader.ReleaseAndGetAddressOf())))
	{
		return false;
	}

	// 頂点データ設定
	D3D11_INPUT_ELEMENT_DESC VertexDesc[]
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"NORMAL" , 0,DXGI_FORMAT_R32G32B32_FLOAT,   0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEXCOORD" , 0, DXGI_FORMAT_R32G32_FLOAT,		 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// インプットレイアウト作成
	if (FAILED(m_Device->CreateInputLayout(VertexDesc, ARRAYSIZE(VertexDesc), vsblob->GetBufferPointer(), vsblob->GetBufferSize(), m_InputLayOut.ReleaseAndGetAddressOf())))
	{
		return false;
	}

	// ピクセルシェーダーコンパイル
	ComPtr<ID3DBlob> psblob;
	CompileShader("Shader/TexturePS.hlsl", "main", "ps_5_0", psblob.ReleaseAndGetAddressOf());

	// ピクセルシェーダー作成
	if (FAILED(m_Device->CreatePixelShader(psblob->GetBufferPointer(), psblob->GetBufferSize(), NULL, m_PixelShader.ReleaseAndGetAddressOf())))
	{
		return false;
	}

	//定数バッファ設定
	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(ConstantBuffer);
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	// 定数バッファ作成
	if (FAILED(m_Device->CreateBuffer(&cbDesc, NULL, m_ConstantBuffer.ReleaseAndGetAddressOf())))
	{
		return false;
	}


	// ワールド座標変換
	XMMATRIX worldMatrix = XMMatrixTranslation(0.0f, 0.0f, 0.0f);

	// ビュー座標
	XMVECTOR eye = XMVectorSet(2.0f, 2.0f, -2.0f, 0.0f);		// 視点位置
	XMVECTOR focus = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);		// 注視点
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);			// 上ベクトル
	XMMATRIX viewMatrix = XMMatrixLookAtLH(eye, focus, up);		// ビュー変換

	// プロジェクション変換
	float    fov = XMConvertToRadians(45.0f);					// 視野角
	float    aspect = m_ViewPort.Width / m_ViewPort.Height;		// アスペクト比
	float    nearZ = 0.1f;										// 近
	float    farZ = 100.0f;										// 遠
	XMMATRIX projMatrix = XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);

	// 各変換行列セット
	ConstantBuffer cb;
	XMStoreFloat4x4(&cb.world, XMMatrixTranspose(worldMatrix));
	XMStoreFloat4x4(&cb.view, XMMatrixTranspose(viewMatrix));
	XMStoreFloat4x4(&cb.projection, XMMatrixTranspose(projMatrix));

	// ライト
	XMVECTOR Light = XMVector3Normalize(XMVectorSet(0.0f, 0.5f, -1.0f, 0.0f));
	XMStoreFloat4(&cb.LightDir, Light);

	m_ImmediateContext->UpdateSubresource(m_ConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

	return true;
}

void DirectX11Wrapper::ObjectUpdate()
{
	static float Angle = 0;
	Angle++;
	if (Angle >= 360)Angle = 0;

	// ワールド座標変換
	XMMATRIX worldMatrix = XMMatrixTranslation(0.0f, 0.0f, 0.0f) ;

	// ビュー座標
	XMVECTOR eye = XMVectorSet(2.0f, 2.0f, -2.0f, 0.0f);		// 視点位置
	XMVECTOR focus = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);		// 注視点
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);			// 上ベクトル
	XMMATRIX viewMatrix = XMMatrixLookAtLH(eye, focus, up);		// ビュー変換

	// プロジェクション変換
	float    fov = XMConvertToRadians(45.0f);					// 視野角
	float    aspect = m_ViewPort.Width / m_ViewPort.Height;		// アスペクト比
	float    nearZ = 0.1f;										// 近
	float    farZ = 100.0f;										// 遠
	XMMATRIX projMatrix = XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);

	// 各変換行列セット
	ConstantBuffer cb;
	XMStoreFloat4x4(&cb.world, XMMatrixTranspose(worldMatrix));
	XMStoreFloat4x4(&cb.view, XMMatrixTranspose(viewMatrix));
	XMStoreFloat4x4(&cb.projection, XMMatrixTranspose(projMatrix));

	// ライト
	XMVECTOR Light = XMVector3Normalize(XMVectorSet(0.0f, 0.5f, -1.0f, 0.0f));
	XMStoreFloat4(&cb.LightDir, Light);

	m_ImmediateContext->UpdateSubresource(m_ConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

}

bool DirectX11Wrapper::CreateTexture()
{
	const int PixelSize = 32;		// 縦横ピクセル数

	// テクスチャの生成
	ComPtr<ID3D11Texture2D> CreateTexture;

	// テクスチャ設定
	D3D11_TEXTURE2D_DESC TexDesc;
	TexDesc.Width = PixelSize;
	TexDesc.Height = PixelSize;
	TexDesc.MipLevels = 1;
	TexDesc.ArraySize = 1;
	TexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	TexDesc.SampleDesc.Count = 1;
	TexDesc.SampleDesc.Quality = 0;
	TexDesc.Usage = D3D11_USAGE_DYNAMIC;
	TexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	TexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	TexDesc.MiscFlags = 0;

	// テクスチャ生成
	if (FAILED(m_Device->CreateTexture2D(&TexDesc, nullptr, &CreateTexture)))
	{
		return false;
	}

	// テクスチャの書き換え
	D3D11_MAPPED_SUBRESOURCE MapSubResouce;;
	m_ImmediateContext->Map(CreateTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MapSubResouce);

	// 乱数生成
	std::random_device seed;
	std::mt19937 random(seed());
	std::uniform_int_distribution<> number(0, 255);		// 0～255

	byte srcData[PixelSize * PixelSize * 4] = { 0 };	// ビットマップを黒で初期化

	// テクスチャデータ作成
	for (int i = 0; i < PixelSize * PixelSize * 4; i += 4)
	{
		srcData[i] = number(random);		//　Red
		srcData[i + 1] = number(random);	//　Green
		srcData[i + 2] = number(random);	//　Blue
	}

	// データ書き込み
	memcpy(MapSubResouce.pData, srcData, sizeof(srcData));
	m_ImmediateContext->Unmap(CreateTexture.Get(), 0);

	// シェーダーリソースビュー設定
	D3D11_SHADER_RESOURCE_VIEW_DESC srv;
	ZeroMemory(&srv, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srv.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv.Texture2D.MipLevels = 1;

	// シェーダーリソースビュー作成
	if (FAILED(m_Device->CreateShaderResourceView(CreateTexture.Get(), &srv, &NoiseTextureResouce)))
	{
		return false;
	}

	//サンプラステート設定
	D3D11_SAMPLER_DESC sd = {};
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	// サンプラーステート作成
	if (FAILED(m_Device->CreateSamplerState(&sd, &m_SamplerState)))
	{
		return false;
	}

	return true;
}

// 描画前処理
void DirectX11Wrapper::BeforeRender()
{
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_ImmediateContext->ClearRenderTargetView(m_RenderTargetView.Get(), clearColor);
	m_ImmediateContext->RSSetViewports(1, &m_ViewPort);
	m_ImmediateContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_ImmediateContext->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), m_DepthStencilView.Get());
}

// 描画後処理
void DirectX11Wrapper::AfterRender()
{
	m_SwapChain->Present(0, 0);
}

