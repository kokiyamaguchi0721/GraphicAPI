#include "DirectX12Wrapper.h"

HRESULT DirectX12Wrapper::Create(HWND hwnd, RECT rc)
{
	HRESULT hr;

#ifdef DEBUG
	EnableDebugLayer();
#endif

	// �t���[�`���[���x����
	D3D_FEATURE_LEVEL Levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

#ifdef DEBUG
	CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(DxgiGactory.ReleaseAndGetAddressOf()));
#else
	// �t�@�N�g���[����
	hr = CreateDXGIFactory1(IID_PPV_ARGS(m_Factory.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		MessageBox(nullptr, "�t�@�N�g���[�̐����Ɏ��s���܂����B", "Error", MB_OK);
		return hr;
	}
#endif

	// �A�_�v�^�[�擾
	std::vector<ComPtr<IDXGIAdapter>> Adapters;
	ComPtr<IDXGIAdapter> tmpAdapter = nullptr;

	for (int i = 0; m_Factory->EnumAdapters(i, tmpAdapter.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		Adapters.push_back(tmpAdapter);
	}

	for (auto adpt : Adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);

		std::wstring strDesc = adesc.Description;

		if (strDesc.find(L"NVIDIA") != std::string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}

	// �œK�ȃ��x���Ńf�o�C�X����
	D3D_FEATURE_LEVEL FeatureLevel;
	bool Flg;
	for (auto lv : Levels)
	{
		if (SUCCEEDED(D3D12CreateDevice(tmpAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(m_Device.ReleaseAndGetAddressOf()))))
		{
			FeatureLevel = lv;
			Flg = true;
			break;
		}
		Flg = false;
	}

	// ���s�����ꍇ
	if (Flg == false)
	{
		MessageBox(nullptr, "�f�o�C�X�̐����Ɏ��s���܂����B", "Error", MB_OK);
		return false;
	}

	// �R�}���h�A���P�[�^�[����
	hr = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocator.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		MessageBox(nullptr, "�R�}���h�A���P�[�^�[�̐����Ɏ��s���܂����B", "Error", MB_OK);
		return hr;
	}


	// �R�}���h���X�g����
	hr = m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), 
		nullptr, IID_PPV_ARGS(m_CommandList.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		MessageBox(nullptr, "�R�}���h�A���P�[�^�[�̐����Ɏ��s���܂����B", "Error", MB_OK);
		return hr;
	}


	// �R�}���h�L���[�ݒ�
	D3D12_COMMAND_QUEUE_DESC CmdQueueDesc = {};
	CmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CmdQueueDesc.NodeMask = 0;
	CmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	CmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	// �R�}���h�L���[����
	hr = m_Device->CreateCommandQueue(&CmdQueueDesc, IID_PPV_ARGS(m_CommandQueue.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		MessageBox(nullptr, "�R�}���h�L���[�̐����Ɏ��s���܂����B", "Error", MB_OK);
		return hr;
	}


	// �X���b�v�`�F�C���ݒ�
	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc = {};
	SwapChainDesc.Width = rc.right;
	SwapChainDesc.Height = rc.bottom;
	SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.Stereo = false;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	SwapChainDesc.BufferCount = 2;
	SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// �X���b�v�`�F�C������
	hr = m_Factory->CreateSwapChainForHwnd(m_CommandQueue.Get(), hwnd, &SwapChainDesc, nullptr,
		nullptr, (IDXGISwapChain1**)m_SwapChain.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		MessageBox(nullptr, "�X���b�v�`�F�C���̐����Ɏ��s���܂����B", "Error", MB_OK);
		return hr;
	}


	// �f�B�X�N���v�^�q�[�v�ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
	HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	HeapDesc.NodeMask = 0;
	HeapDesc.NumDescriptors = 2;
	HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	// �f�B�X�N���v�^�[�q�[�v����
	hr = m_Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(RTVHeaps.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		MessageBox(nullptr, "�f�B�X�N���v�^�q�[�v�̐����Ɏ��s���܂����B", "Error", MB_OK);
		return hr;
	}


	// �����_�[�^�[�Q�b�g�r���[�ݒ�
	{D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	m_SwapChain->GetDesc(&swcDesc);
	BackBuffers.resize(swcDesc.BufferCount);

	// �擪�A�h���X�擾
	D3D12_CPU_DESCRIPTOR_HANDLE Handle = RTVHeaps->GetCPUDescriptorHandleForHeapStart();
	for (auto idx = 0; idx < swcDesc.BufferCount; ++idx)
	{
		m_SwapChain->GetBuffer(static_cast<UINT>(idx), IID_PPV_ARGS(&BackBuffers[idx]));
		// �����_�[�^�[�Q�b�g�r���[����
		m_Device->CreateRenderTargetView(BackBuffers[idx].Get(), &rtvDesc, Handle);
		Handle.ptr += m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
	}

	// �t�F���X����
	hr = m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		MessageBox(nullptr, "�t�F���X�̐����Ɏ��s���܂����B", "Error", MB_OK);
		return hr;
	}

	// �r���[�|�[�g�ݒ�
	ViewPort.Width = (FLOAT)rc.right;		// ��ʂ̉���
	ViewPort.Height = (FLOAT)rc.bottom;		// ��ʂ̏c��
	ViewPort.TopLeftX = 0;					// ����X���W
	ViewPort.TopLeftY = 0;					// ����Y���W
	ViewPort.MaxDepth = 1.0f;				// �[�x�̍ő�l
	ViewPort.MinDepth = 0.0f;				// �[�x�̍ŏ��l

	// �V�U�[��`�ݒ�
	ScissorRect.top = 0;								// ����W
	ScissorRect.left = 0;								// �����W
	ScissorRect.right = ScissorRect.left + rc.right;			// �E���W
	ScissorRect.bottom = ScissorRect.top + rc.bottom;		// �����W

	// �[�x�o�b�t�@�ݒ�
	D3D12_RESOURCE_DESC DepthResDesc = {};
	DepthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	DepthResDesc.Width = rc.right;
	DepthResDesc.Height = rc.bottom;
	DepthResDesc.DepthOrArraySize = 1;
	DepthResDesc.Format = DXGI_FORMAT_D32_FLOAT;
	DepthResDesc.SampleDesc.Count = 1;
	DepthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// �[�x�p�q�[�v�v���p�e�B
	D3D12_HEAP_PROPERTIES DepthHeapProp = {};
	DepthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	DepthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	DepthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_CLEAR_VALUE DepthClearValue = {};
	DepthClearValue.DepthStencil.Depth = 1.0f;
	DepthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	// �[�x�o�b�t�@����
	hr = m_Device->CreateCommittedResource(&DepthHeapProp, D3D12_HEAP_FLAG_NONE, &DepthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, &DepthClearValue, IID_PPV_ARGS(DepthBuffer.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		MessageBox(nullptr, "�[�x�o�b�t�@�̐����Ɏ��s���܂����B", "Error", MB_OK);
		return hr;
	}

	// �[�x�p�f�B�X�N���v�^�[�q�[�v�ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	// �[�x�p�f�B�X�N���v�^�[�q�[�v����
	hr = m_Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(dsvHeap.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		MessageBox(nullptr, "�[�x�p�f�B�X�N���v�^�[�q�[�v�̐����Ɏ��s���܂����B", "Error", MB_OK);
		return hr;
	}
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	// �[�x�p�f�B�X�N���v�^�[�q�[�v����
	m_Device->CreateDepthStencilView(DepthBuffer.Get(), &dsvDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());

	return S_OK;
}

void DirectX12Wrapper::Release()
{
}

// �`��O����
void DirectX12Wrapper::BeforeRender()
{
	BuckBuffIdx = m_SwapChain->GetCurrentBackBufferIndex();
	SetResouceBarrier(BackBuffers[BuckBuffIdx].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	auto rtvH = RTVHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += static_cast<ULONG_PTR>(BuckBuffIdx * m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));

	// �[�x�o�b�t�@�擾
	auto dsvH = dsvHeap->GetCPUDescriptorHandleForHeapStart();

	m_CommandList->OMSetRenderTargets(1, &rtvH, false, &dsvH);

	//�@��ʃN���A
	float clearColor[] = { 0.0f,1.0f,0.0f,1.0f };
	m_CommandList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
	m_CommandList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	m_CommandList->RSSetViewports(1, &ViewPort);			// �r���[�|�[�g
	m_CommandList->RSSetScissorRects(1, &ScissorRect);	// �V�U�[��`
}

// �`��㏈��
void DirectX12Wrapper::AfterRender()
{
	SetResouceBarrier(BackBuffers[BuckBuffIdx].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	// ���߃N���[�Y
	m_CommandList->Close();

	// �R�}���h���X�g�̎��s
	ID3D12CommandList* cmdLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(1, cmdLists);

	// �t�F���X����
	WaitForCommandQueue();

	// �R�}���h���Z�b�g
	m_CommandAllocator->Reset();
	m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);

	// ��ʐ؂�ւ�
	m_SwapChain->Present(1, 0);
}

// ���\�[�X�o���A
void DirectX12Wrapper::SetResouceBarrier(ID3D12Resource* Resouce, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After)
{
	// ���\�[�X�o���A�ݒ�
	D3D12_RESOURCE_BARRIER BarrierDesc;
	ZeroMemory(&BarrierDesc, sizeof(BarrierDesc));
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Transition.pResource = Resouce;
	BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	BarrierDesc.Transition.StateBefore = Before;
	BarrierDesc.Transition.StateAfter = After;

	// ���\�[�X�o���A�Z�b�g
	m_CommandList->ResourceBarrier(1, &BarrierDesc);
}

// �t�F���X����
void DirectX12Wrapper::WaitForCommandQueue()
{
	m_CommandQueue->Signal(m_Fence.Get(), ++FenceVal);

	// GPU�̏������I����Ă��Ȃ���
	if (m_Fence->GetCompletedValue() != FenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);		// �C�x���g�n���h���擾
		m_Fence->SetEventOnCompletion(FenceVal, event);					// �C�x���g�Z�b�g
		WaitForSingleObject(event, INFINITE);							// �C�x���g����������܂ő҂�
		CloseHandle(event);												// �C�x���g�I��
	}
}

bool DirectX12Wrapper::PolygonInit()
{
	// ���_���X�g
	Vertex VertexList[]{
	{ { -0.5f,  0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f }, {0.0f,0.0f}},
	{ {  0.5f, -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f }, {1.0f,0.0f}},
	{ { -0.5f, -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f }, {0.0f,1.0f}},
	{ {  0.5f,  0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f }, {1.0f,1.0f}}
	};


	HRESULT hr;

	// ���[�g�V�O�l�`���ݒ�
	D3D12_ROOT_SIGNATURE_DESC RootSignatureDesc;
	ZeroMemory(&RootSignatureDesc, sizeof(RootSignatureDesc));
	RootSignatureDesc.NumParameters = 0;
	RootSignatureDesc.pParameters = nullptr;
	RootSignatureDesc.pStaticSamplers = nullptr;
	RootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// ���[�g�V�O�l�`������
	ComPtr<ID3DBlob> RootSignatureBlob, ErrorBlob;
	hr = D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, RootSignatureBlob.ReleaseAndGetAddressOf(), ErrorBlob.ReleaseAndGetAddressOf());
	hr = m_Device->CreateRootSignature(0, RootSignatureBlob->GetBufferPointer(), RootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(m_RootSignature.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) return false;


	// ���_�V�F�[�_�[�ǂݍ���
	ComPtr<ID3DBlob> vsBlob;
	CompileShader("Shader/PolygonVS.hlsl", "main", "vs_5_0", vsBlob.ReleaseAndGetAddressOf());

	// �s�N�Z���V�F�[�_�[�ǂݍ���
	ComPtr<ID3DBlob> psBlob;
	CompileShader("Shader/BasicPS.hlsl", "main", "ps_5_0", psBlob.ReleaseAndGetAddressOf());


	// ���_���C�A�E�g�ݒ�
	D3D12_INPUT_ELEMENT_DESC VertexDesc[]
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT   ,0,0                           ,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"COLOR"   ,0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"NORMAL" , 0,DXGI_FORMAT_R32G32B32_FLOAT,   0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,		 0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
	};


	// �p�C�v���C���X�e�[�g�ݒ�
	D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineStateDesc;
	ZeroMemory(&PipelineStateDesc, sizeof(PipelineStateDesc));

	// ���_�V�F�[�_�[
	PipelineStateDesc.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	PipelineStateDesc.VS.BytecodeLength = vsBlob->GetBufferSize();

	// �s�N�Z���V�F�[�_�[
	PipelineStateDesc.PS.pShaderBytecode = psBlob->GetBufferPointer();
	PipelineStateDesc.PS.BytecodeLength = psBlob->GetBufferSize();

	PipelineStateDesc.SampleDesc.Count = 1;
	PipelineStateDesc.SampleMask = UINT_MAX;

	// �C���v�b�g���C�A�E�g
	PipelineStateDesc.InputLayout.pInputElementDescs = VertexDesc;
	PipelineStateDesc.InputLayout.NumElements = _countof(VertexDesc);

	// ���[�g�V�O�l�`��
	PipelineStateDesc.pRootSignature = m_RootSignature.Get();

	// �����_�[�^�[�Q�b�g�r���[
	PipelineStateDesc.NumRenderTargets = 1;
	PipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	// �g�|���W�[�^�C�v
	PipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// ���X�^���C�U�[
	PipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	PipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	PipelineStateDesc.RasterizerState.DepthClipEnable = TRUE;
	PipelineStateDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	// �u�����h�ݒ�
	D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineStateDescForBlend;
	for (int i = 0; i < _countof(PipelineStateDescForBlend.BlendState.RenderTarget); ++i)
	{
		PipelineStateDesc.BlendState.RenderTarget[i].BlendEnable = FALSE;
		PipelineStateDesc.BlendState.RenderTarget[i].SrcBlend = D3D12_BLEND_ONE;
		PipelineStateDesc.BlendState.RenderTarget[i].DestBlend = D3D12_BLEND_ZERO;
		PipelineStateDesc.BlendState.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
		PipelineStateDesc.BlendState.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
		PipelineStateDesc.BlendState.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ZERO;
		PipelineStateDesc.BlendState.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		PipelineStateDesc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}

	// �[�x�X�e���V��
	PipelineStateDesc.DepthStencilState.DepthEnable = FALSE;

	// �p�C�v���C���X�e�[�g����
	hr = m_Device->CreateGraphicsPipelineState(&PipelineStateDesc, IID_PPV_ARGS(m_PipelineState.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) return false;

	return true;
}
