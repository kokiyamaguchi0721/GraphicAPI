#include "DirectX12Wrapper.h"

HRESULT DirectX12Wrapper::Create(HWND hwnd, RECT rc)
{
	HRESULT hr;

	// �f�o�b�O
#if defined(DEBUG) || defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debug;
		auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(debug.ReleaseAndGetAddressOf()));

		// �f�o�b�O���C���[��L����.
		if (SUCCEEDED(hr))
		{
			debug->EnableDebugLayer();
		}
	}
#endif

	// �f�o�C�X����
	hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_Device.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) return hr;

	// �R�}���h�L���[�ݒ�
	D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
	QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	QueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	QueueDesc.NodeMask = 0;

	// �R�}���h�L���[����
	hr = m_Device->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(m_CmdQueue.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) return hr;

	// DXGI�t�@�N�g������
	ComPtr<IDXGIFactory4> Factory = nullptr;
	hr = CreateDXGIFactory1(IID_PPV_ARGS(Factory.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) return hr;

	// �X���b�v�`�F�C���ݒ�
	DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
	SwapChainDesc.BufferDesc.Width = rc.right;
	SwapChainDesc.BufferDesc.Height = rc.bottom;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferCount = FrameCount;
	SwapChainDesc.OutputWindow = hwnd;
	SwapChainDesc.Windowed = TRUE;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// �X���b�v�`�F�C������
	ComPtr<IDXGISwapChain> pSwapChain;
	hr = Factory->CreateSwapChain(m_CmdQueue.Get(), &SwapChainDesc, pSwapChain.ReleaseAndGetAddressOf());
	if (FAILED(hr))	return hr;


	// IDXGISwapChain3���擾
	hr = pSwapChain.As(&m_SwapChain);
	if (FAILED(hr)) return hr;

	// �o�b�N�o�b�t�@�ԍ��擾
	m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

	// ���
	Factory.Reset();
	pSwapChain.Reset();

	// �R�}���h�A���P�[�^����
	for (auto i = 0; i < FrameCount; ++i)
	{
		hr = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CmdAllocator[i].ReleaseAndGetAddressOf()));
		if (FAILED(hr)) return hr;
	}

	// �R�}���h���X�g����
	hr = m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CmdAllocator[m_FrameIndex].Get(),
		nullptr, IID_PPV_ARGS(m_CmdList.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) return hr;

	// RTV�ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc = {};
	RTVHeapDesc.NumDescriptors = FrameCount;
	RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RTVHeapDesc.NodeMask = 0;

	// �f�B�X�N���v�^�q�[�v�𐶐�.
	hr = m_Device->CreateDescriptorHeap(&RTVHeapDesc, IID_PPV_ARGS(m_HeapRTV.ReleaseAndGetAddressOf()));
	if (FAILED(hr))	return hr;


	auto handle = m_HeapRTV->GetCPUDescriptorHandleForHeapStart();
	auto incrementSize = m_Device
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (auto i = 0u; i < FrameCount; ++i)
	{
		hr = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_ColorBuffer[i].ReleaseAndGetAddressOf()));
		if (FAILED(hr))	return hr;


		D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
		viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipSlice = 0;
		viewDesc.Texture2D.PlaneSlice = 0;

		// �����_�[�^�[�Q�b�g�r���[�̐���.
		m_Device->CreateRenderTargetView(m_ColorBuffer[i].Get(), &viewDesc, handle);

		m_HandleRTV[i] = handle;
		handle.ptr += incrementSize;
	}

	// �t�F���X�J�E���^�[�����Z�b�g.
	for (auto i = 0u; i < FrameCount; ++i)
	{
		m_FenceCounter[i] = 0;
	}

	// �t�F���X�̐���.
	hr = m_Device->CreateFence(
		m_FenceCounter[m_FrameIndex],
		D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(m_Fence.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) return hr;

	m_FenceCounter[m_FrameIndex]++;

	// �C�x���g�̐���.
	m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_FenceEvent == nullptr) return hr;

	// �R�}���h���X�g�����.
	m_CmdList->Close();

	// �r���[�|�[�g
	m_ViewPort.TopLeftX = 0;
	m_ViewPort.TopLeftY = 0;
	m_ViewPort.Width = static_cast<float>(rc.right);
	m_ViewPort.Height = static_cast<float>(rc.	bottom);
	m_ViewPort.MinDepth = 0.0f;
	m_ViewPort.MaxDepth = 1.0f;

	// �V�U�[��`
	m_Scissor.left = 0;
	m_Scissor.right = rc.right;
	m_Scissor.top = 0;
	m_Scissor.bottom = rc.bottom;

	// �[�x�X�e���V���o�b�t�@�ݒ�
	D3D12_HEAP_PROPERTIES DepthHeapProp = {};
	DepthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	DepthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	DepthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	DepthHeapProp.CreationNodeMask = 1;
	DepthHeapProp.VisibleNodeMask = 1;

	// �[�x�p���\�[�X�ݒ�
	D3D12_RESOURCE_DESC DepthResDesc = {};
	DepthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	DepthResDesc.Alignment = 0;
	DepthResDesc.Width = rc.right;
	DepthResDesc.Height = rc.bottom;
	DepthResDesc.DepthOrArraySize = 1;
	DepthResDesc.MipLevels = 1;
	DepthResDesc.Format = DXGI_FORMAT_D32_FLOAT;
	DepthResDesc.SampleDesc.Count = 1;
	DepthResDesc.SampleDesc.Quality = 0;
	DepthResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	DepthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// �[�x�o�b�t�@�N���A�l
	D3D12_CLEAR_VALUE ClearValue;
	ClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	ClearValue.DepthStencil.Depth = 1.0f;
	ClearValue.DepthStencil.Stencil = 0;


	// �[�x�o�b�t�@����
	hr = m_Device->CreateCommittedResource(&DepthHeapProp,D3D12_HEAP_FLAG_NONE,&DepthResDesc,D3D12_RESOURCE_STATE_DEPTH_WRITE,&ClearValue,IID_PPV_ARGS(m_DepthBuffer.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) return hr;

	// �f�B�X�N���v�^�q�[�v�ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC DepthHeapDesc = {};
	DepthHeapDesc.NumDescriptors = 1;
	DepthHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DepthHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DepthHeapDesc.NodeMask = 0;

	// �f�B�X�N���v�^�q�[�v����
	hr = m_Device->CreateDescriptorHeap(&DepthHeapDesc,IID_PPV_ARGS(m_HeapDSV.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) return hr;

	handle = m_HeapDSV->GetCPUDescriptorHandleForHeapStart();
	incrementSize = m_Device
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	D3D12_DEPTH_STENCIL_VIEW_DESC DepthViewDesc = {};
	DepthViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	DepthViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DepthViewDesc.Texture2D.MipSlice = 0;
	DepthViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	// DSV����
	m_Device->CreateDepthStencilView(m_DepthBuffer.Get(), &DepthViewDesc, handle);

	m_HandleDSV = handle;

	return S_OK;
}

void DirectX12Wrapper::Release()
{
	//for (auto Idx = 0; Idx < FrameCount; ++Idx)
	//{
	//	if (m_ConstantBuffer[Idx].Get() != nullptr)
	//	{
	//		m_ConstantBuffer[Idx]->Unmap(0, nullptr);
	//		memset(&m_CBView[Idx], 0, sizeof(m_ConstantBuffer[Idx]));
	//	}
	//	m_ConstantBuffer[Idx].Reset();
	//}

	// �t�F���X����
	WaitGPU();

	// �C�x���g�j��
	if (m_FenceEvent != nullptr)
	{
		CloseHandle(m_FenceEvent);
		m_FenceEvent = nullptr;
	}
}

void DirectX12Wrapper::WaitGPU()
{
	assert(m_CmdQueue != nullptr);
	assert(m_Fence != nullptr);
	assert(m_FenceEvent != nullptr);

	// �V�O�i������.
	m_CmdQueue->Signal(m_Fence.Get(), m_FenceCounter[m_FrameIndex]);

	// �������ɃC�x���g��ݒ肷��..
	m_Fence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);

	// �ҋ@����.
	WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);

	// �J�E���^�[�𑝂₷.
	m_FenceCounter[m_FrameIndex]++;
}

// �`��O����
void DirectX12Wrapper::ObjectDraw()
{
	m_CmdList->SetGraphicsRootSignature(m_RootSignature.Get());
	m_CmdList->SetDescriptorHeaps(1, m_HeapCBV.GetAddressOf());
	m_CmdList->SetGraphicsRootConstantBufferView(0, m_CBView[m_FrameIndex].Desc.BufferLocation);
	m_CmdList->SetPipelineState(m_PSO.Get());

	m_CmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_CmdList->IASetVertexBuffers(0, 1, &m_VBView);
	m_CmdList->IASetIndexBuffer(&m_IBView);             

	m_CmdList->DrawIndexedInstanced(m_IndexNum, 1, 0, 0, 0);  
}

void DirectX12Wrapper::BeforeRender()
{
	// �X�V����
	{
		m_RotateAngle += 0.025f;
		m_CBView[m_FrameIndex].pBuffer->World = DirectX::XMMatrixRotationY(m_RotateAngle);
	}
	// �R�}���h���͊J�n
	m_CmdAllocator[m_FrameIndex]->Reset();
	m_CmdList->Reset(m_CmdAllocator[m_FrameIndex].Get(), nullptr);

	SetResouceBarrier(m_ColorBuffer[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);


	// �����_�[�Q�b�g�ݒ�
	m_CmdList->OMSetRenderTargets(1, &m_HandleRTV[m_FrameIndex], false, &m_HandleDSV);

	// �N���A�J���[�ݒ�
	float clearColor[] = { 0.25f, 0.25f, 0.25f, 1.0f };

	// �����_�[�^�[�Q�b�g�r���[���N���A
	m_CmdList->ClearRenderTargetView(m_HandleRTV[m_FrameIndex], clearColor, 0, nullptr);

	// �[�x�X�e���V���r���[���N���A
	m_CmdList->ClearDepthStencilView(m_HandleDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	m_CmdList->RSSetViewports(1, &m_ViewPort);
	m_CmdList->RSSetScissorRects(1, &m_Scissor);

}

void DirectX12Wrapper::AfterRender()
{
	SetResouceBarrier(m_ColorBuffer[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	// �R�}���h�̋L�^���I��
	m_CmdList->Close();

	// �R�}���h�����s
	ID3D12CommandList* ppCmdLists[] = { m_CmdList.Get() };
	m_CmdQueue->ExecuteCommandLists(1, ppCmdLists);


	// ��ʂɕ\��
	m_SwapChain->Present(1, 0);

	// �V�O�i������
	const auto currentValue = m_FenceCounter[m_FrameIndex];
	m_CmdQueue->Signal(m_Fence.Get(), currentValue);

	// �o�b�N�o�b�t�@�ԍ����X�V
	m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

	// �t�F���X����
	if (m_Fence->GetCompletedValue() < m_FenceCounter[m_FrameIndex])
	{
		m_Fence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);
		WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
	}

	// �t���[���J�E���^�[����
	m_FenceCounter[m_FrameIndex] = currentValue + 1;
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
	m_CmdList->ResourceBarrier(1, &BarrierDesc);
}


bool DirectX12Wrapper::PolygonInit()
{
	// ���_�f�[�^
	Vertex VertexList[]{
		{ { -0.5f,  0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f }, {0.0f,0.0f}},
		{ {  0.5f, -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f }, {1.0f,0.0f}},
		{ { -0.5f, -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f }, {0.0f,1.0f}},
		{ {  0.5f,  0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f }, {1.0f,1.0f}}
	};


	// ���_�o�b�t�@�ݒ�
	D3D12_HEAP_PROPERTIES VBProp = {};
	VBProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	VBProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	VBProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	VBProp.CreationNodeMask = 1;
	VBProp.VisibleNodeMask = 1;

	// ���\�[�X�ݒ�
	D3D12_RESOURCE_DESC VBResDesc = {};
	VBResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	VBResDesc.Alignment = 0;
	VBResDesc.Width = sizeof(VertexList);
	VBResDesc.Height = 1;
	VBResDesc.DepthOrArraySize = 1;
	VBResDesc.MipLevels = 1;
	VBResDesc.Format = DXGI_FORMAT_UNKNOWN;
	VBResDesc.SampleDesc.Count = 1;
	VBResDesc.SampleDesc.Quality = 0;
	VBResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	VBResDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// ���_�o�b�t�@����
	auto hr = m_Device->CreateCommittedResource(&VBProp, D3D12_HEAP_FLAG_NONE,
		&VBResDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_VertexBuffer.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) return false;

	// ���_�f�[�^�]���J�n
	void* SendVertexData = nullptr;
	hr = m_VertexBuffer->Map(0, nullptr, &SendVertexData);
	if (FAILED(hr)) hr;

	// �]��
	memcpy(SendVertexData, VertexList, sizeof(VertexList));

	// ���_�f�[�^�]���I��
	m_VertexBuffer->Unmap(0, nullptr);

	// ���_�o�b�t�@�[�r���[�ݒ�
	m_VBView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
	m_VBView.SizeInBytes = static_cast<UINT>(sizeof(VertexList));
	m_VBView.StrideInBytes = static_cast<UINT>(sizeof(Vertex));

	// �C���f�b�N�X���X�g
	uint32_t Indices[] = 
	{ 
		0, 1, 2,
		0, 3, 1, 
	};

	m_IndexNum = ARRAYSIZE(Indices);

	// �C���f�b�N�X�o�b�t�@�ݒ�
	D3D12_HEAP_PROPERTIES IBProp = {};
	IBProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	IBProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	IBProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	IBProp.CreationNodeMask = 1;
	IBProp.VisibleNodeMask = 1;

	// ���\�[�X�̐ݒ�
	D3D12_RESOURCE_DESC IBResDesc = {};
	IBResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	IBResDesc.Alignment = 0;
	IBResDesc.Width = sizeof(Indices);
	IBResDesc.Height = 1;
	IBResDesc.DepthOrArraySize = 1;
	IBResDesc.MipLevels = 1;
	IBResDesc.Format = DXGI_FORMAT_UNKNOWN;
	IBResDesc.SampleDesc.Count = 1;
	IBResDesc.SampleDesc.Quality = 0;
	IBResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	IBResDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// �C���f�b�N�X�o�b�t�@����
	hr = m_Device->CreateCommittedResource(&IBProp, D3D12_HEAP_FLAG_NONE,
		&IBResDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_IndexBuffer.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) return false;

	// �C���f�b�N�X�f�[�^�]���J�n
	void* SendIndexData = nullptr;
	hr = m_IndexBuffer->Map(0, nullptr, &SendIndexData);
	if (FAILED(hr)) return hr;

	// �]��
	memcpy(SendIndexData, Indices, sizeof(Indices));

	// �C���f�b�N�X�f�[�^�]���I��
	m_IndexBuffer->Unmap(0, nullptr);

	// �C���f�b�N�X�o�b�t�@�r���[�ݒ�
	m_IBView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
	m_IBView.Format = DXGI_FORMAT_R32_UINT;
	m_IBView.SizeInBytes = sizeof(Indices);

	// �萔�o�b�t�@�p�f�B�X�N���v�^�q�[�v�ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC CBDesc = {};
	CBDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	CBDesc.NumDescriptors = 1 * FrameCount;
	CBDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	CBDesc.NodeMask = 0;

	// �萔�o�b�t�@�p�f�B�X�N���v�^�q�[�v����
	hr = m_Device->CreateDescriptorHeap(&CBDesc, IID_PPV_ARGS(m_HeapCBV.GetAddressOf()));
	if (FAILED(hr)) return false;

	// �萔�o�b�t�@�ݒ�
	D3D12_HEAP_PROPERTIES CBProp = {};
	CBProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	CBProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	CBProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	CBProp.CreationNodeMask = 1;
	CBProp.VisibleNodeMask = 1;

	// ���\�[�X�̐ݒ�.
	D3D12_RESOURCE_DESC CBResDesc = {};
	CBResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	CBResDesc.Alignment = 0;
	CBResDesc.Width = sizeof(Transform);
	CBResDesc.Height = 1;
	CBResDesc.DepthOrArraySize = 1;
	CBResDesc.MipLevels = 1;
	CBResDesc.Format = DXGI_FORMAT_UNKNOWN;
	CBResDesc.SampleDesc.Count = 1;
	CBResDesc.SampleDesc.Quality = 0;
	CBResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	CBResDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// �C���N�������g�p�T�C�Y�擾
	auto incrementSize = m_Device
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (auto Idx = 0; Idx < FrameCount; ++Idx)
	{
		hr = m_Device->CreateCommittedResource(&CBProp, D3D12_HEAP_FLAG_NONE,
			&CBResDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_ConstantBuffer[Idx].ReleaseAndGetAddressOf()));
		if (FAILED(hr)) return false;

		auto Address = m_ConstantBuffer[Idx]->GetGPUVirtualAddress();
		auto HandleCPU = m_HeapCBV->GetCPUDescriptorHandleForHeapStart();
		auto HandleGPU = m_HeapCBV->GetGPUDescriptorHandleForHeapStart();

		HandleCPU.ptr += incrementSize * Idx;
		HandleGPU.ptr += incrementSize * Idx;

		// �萔�o�b�t�@�r���[�ݒ�
		m_CBView[Idx].HandleCPU = HandleCPU;
		m_CBView[Idx].HandleGPU = HandleGPU;
		m_CBView[Idx].Desc.BufferLocation = Address;
		m_CBView[Idx].Desc.SizeInBytes = sizeof(Transform);

		// �萔�o�b�t�@�r���[����
		m_Device->CreateConstantBufferView(&m_CBView[Idx].Desc, HandleCPU);

		// �萔�f�[�^�]���J�n
		hr = m_ConstantBuffer[Idx]->Map(0, nullptr, reinterpret_cast<void**>(&m_CBView[Idx].pBuffer));
		if (FAILED(hr))
		{
			return false;
		}

		auto eyePos = DirectX::XMVectorSet(0.0f, 0.0f, 5.0f, 0.0f);
		auto targetPos = DirectX::XMVectorZero();
		auto upward = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		auto fovY = DirectX::XMConvertToRadians(37.5f);
		auto aspect = static_cast<float>(1200) / static_cast<float>(800);

		// �ϊ��s��̐ݒ�.
		m_CBView[Idx].pBuffer->World = DirectX::XMMatrixIdentity();
		m_CBView[Idx].pBuffer->View = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
		m_CBView[Idx].pBuffer->Proj = DirectX::XMMatrixPerspectiveFovRH(fovY, aspect, 1.0f, 1000.0f);
	}

	auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	// ���[�g�p�����[�^�ݒ�
	D3D12_ROOT_PARAMETER RootParam = {};
	RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	RootParam.Descriptor.ShaderRegister = 0;
	RootParam.Descriptor.RegisterSpace = 0;
	RootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// ���[�g�V�O�j�`���ݒ�
	D3D12_ROOT_SIGNATURE_DESC RootDesc = {};
	RootDesc.NumParameters = 1;
	RootDesc.NumStaticSamplers = 0;
	RootDesc.pParameters = &RootParam;
	RootDesc.pStaticSamplers = nullptr;
	RootDesc.Flags = flag;

	ComPtr<ID3DBlob> pBlob;
	ComPtr<ID3DBlob> pErrorBlob;

	// �V���A���C�Y
	hr = D3D12SerializeRootSignature(&RootDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
		pBlob.GetAddressOf(), pErrorBlob.ReleaseAndGetAddressOf());
	if (FAILED(hr)) return false;


	// ���[�g�V�O�j�`���𐶐�
	hr = m_Device->CreateRootSignature(0, pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(), IID_PPV_ARGS(m_RootSignature.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) return false;

	D3D12_INPUT_ELEMENT_DESC VertexLayout[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{ "COLOR"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{"NORMAL" , 0,DXGI_FORMAT_R32G32B32_FLOAT,   0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TEXCOORD" , 0, DXGI_FORMAT_R32G32_FLOAT,		 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	// �����_�[�^�[�Q�b�g�̃u�����h�ݒ�.
	D3D12_RENDER_TARGET_BLEND_DESC RTBDesc = {
		FALSE, FALSE,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL
	};

	// �u�����h�X�e�[�g�̐ݒ�.
	D3D12_BLEND_DESC BSDesc;
	BSDesc.AlphaToCoverageEnable = true;
	BSDesc.IndependentBlendEnable = FALSE;
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
	{
		BSDesc.RenderTarget[i] = RTBDesc;
	}

	// �[�x�X�e���V���X�e�[�g�ݒ�
	D3D12_DEPTH_STENCIL_DESC DepStencilDesc = {};
	DepStencilDesc.DepthEnable = false;
	DepStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	DepStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	DepStencilDesc.StencilEnable = false;

	ComPtr<ID3DBlob> VSBlob;
	ComPtr<ID3DBlob> PSBlob;

	// ���_�V�F�[�_�[�ǂݍ���
	if (!CompileShader("Shader/SimpleVS.hlsl", "main", "vs_5_0", VSBlob.ReleaseAndGetAddressOf()))
	{
		return false;
	}

	// �s�N�Z���V�F�[�_�[�ǂݍ���
	if (!CompileShader("Shader/SimplePS.hlsl", "main", "ps_5_0", PSBlob.ReleaseAndGetAddressOf()))
	{
		return false;
	}

	// �p�C�v���C���X�e�[�g�ݒ�
	D3D12_GRAPHICS_PIPELINE_STATE_DESC PipeStateDesc = {};
	PipeStateDesc.InputLayout = { VertexLayout, _countof(VertexLayout) };
	PipeStateDesc.pRootSignature = m_RootSignature.Get();
	PipeStateDesc.VS.BytecodeLength = VSBlob->GetBufferSize();
	PipeStateDesc.VS.pShaderBytecode = VSBlob->GetBufferPointer();
	PipeStateDesc.PS.BytecodeLength = PSBlob->GetBufferSize();
	PipeStateDesc.PS.pShaderBytecode = PSBlob->GetBufferPointer();

	// ���X�^���C�U�[�X�e�[�g�ݒ�
	D3D12_RASTERIZER_DESC RasDesc;
	RasDesc.FillMode = D3D12_FILL_MODE_SOLID;
	RasDesc.CullMode = D3D12_CULL_MODE_BACK;
	RasDesc.FrontCounterClockwise = FALSE;
	RasDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	RasDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	RasDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	RasDesc.DepthClipEnable = FALSE;
	RasDesc.MultisampleEnable = FALSE;
	RasDesc.AntialiasedLineEnable = FALSE;
	RasDesc.ForcedSampleCount = 0;
	RasDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	PipeStateDesc.RasterizerState = RasDesc;
	PipeStateDesc.BlendState = BSDesc;
	PipeStateDesc.DepthStencilState = DepStencilDesc;
	PipeStateDesc.SampleMask = UINT_MAX;
	PipeStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	PipeStateDesc.NumRenderTargets = 1;
	PipeStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	PipeStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	PipeStateDesc.SampleDesc.Count = 1;
	PipeStateDesc.SampleDesc.Quality = 0;

	// �p�C�v���C���X�e�[�g����
	hr = m_Device->CreateGraphicsPipelineState(&PipeStateDesc,IID_PPV_ARGS(m_PSO.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) return false;

	return true;
}

bool DirectX12Wrapper::CubeInit()
{
	// ���_�f�[�^
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


	// ���_�o�b�t�@�ݒ�
	D3D12_HEAP_PROPERTIES VBProp = {};
	VBProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	VBProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	VBProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	VBProp.CreationNodeMask = 1;
	VBProp.VisibleNodeMask = 1;

	// ���\�[�X�ݒ�
	D3D12_RESOURCE_DESC VBResDesc = {};
	VBResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	VBResDesc.Alignment = 0;
	VBResDesc.Width = sizeof(VertexList);
	VBResDesc.Height = 1;
	VBResDesc.DepthOrArraySize = 1;
	VBResDesc.MipLevels = 1;
	VBResDesc.Format = DXGI_FORMAT_UNKNOWN;
	VBResDesc.SampleDesc.Count = 1;
	VBResDesc.SampleDesc.Quality = 0;
	VBResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	VBResDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// ���_�o�b�t�@����
	auto hr = m_Device->CreateCommittedResource(&VBProp, D3D12_HEAP_FLAG_NONE,
		&VBResDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_VertexBuffer.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) return false;

	// ���_�f�[�^�]���J�n
	void* SendVertexData = nullptr;
	hr = m_VertexBuffer->Map(0, nullptr, &SendVertexData);
	if (FAILED(hr)) hr;

	// �]��
	memcpy(SendVertexData, VertexList, sizeof(VertexList));

	// ���_�f�[�^�]���I��
	m_VertexBuffer->Unmap(0, nullptr);

	// ���_�o�b�t�@�[�r���[�ݒ�
	m_VBView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
	m_VBView.SizeInBytes = static_cast<UINT>(sizeof(VertexList));
	m_VBView.StrideInBytes = static_cast<UINT>(sizeof(Vertex));

	// �C���f�b�N�X���X�g
	uint32_t Indices[] =
	{
		 0,  1,  2,     3,  2,  1,
		 4,  5,  6,     7,  6,  5,
		 8,  9, 10,    11, 10,  9,
		12, 13, 14,    15, 14, 13,
		16, 17, 18,    19, 18, 17,
		20, 21, 22,    23, 22, 21,
	};

	m_IndexNum = ARRAYSIZE(Indices);

	// �C���f�b�N�X�o�b�t�@�ݒ�
	D3D12_HEAP_PROPERTIES IBProp = {};
	IBProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	IBProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	IBProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	IBProp.CreationNodeMask = 1;
	IBProp.VisibleNodeMask = 1;

	// ���\�[�X�̐ݒ�
	D3D12_RESOURCE_DESC IBResDesc = {};
	IBResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	IBResDesc.Alignment = 0;
	IBResDesc.Width = sizeof(Indices);
	IBResDesc.Height = 1;
	IBResDesc.DepthOrArraySize = 1;
	IBResDesc.MipLevels = 1;
	IBResDesc.Format = DXGI_FORMAT_UNKNOWN;
	IBResDesc.SampleDesc.Count = 1;
	IBResDesc.SampleDesc.Quality = 0;
	IBResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	IBResDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// �C���f�b�N�X�o�b�t�@����
	hr = m_Device->CreateCommittedResource(&IBProp, D3D12_HEAP_FLAG_NONE,
		&IBResDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_IndexBuffer.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) return false;

	// �C���f�b�N�X�f�[�^�]���J�n
	void* SendIndexData = nullptr;
	hr = m_IndexBuffer->Map(0, nullptr, &SendIndexData);
	if (FAILED(hr)) return hr;

	// �]��
	memcpy(SendIndexData, Indices, sizeof(Indices));

	// �C���f�b�N�X�f�[�^�]���I��
	m_IndexBuffer->Unmap(0, nullptr);

	// �C���f�b�N�X�o�b�t�@�r���[�ݒ�
	m_IBView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
	m_IBView.Format = DXGI_FORMAT_R32_UINT;
	m_IBView.SizeInBytes = sizeof(Indices);

	// �萔�o�b�t�@�p�f�B�X�N���v�^�q�[�v�ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC CBDesc = {};
	CBDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	CBDesc.NumDescriptors = 1 * FrameCount;
	CBDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	CBDesc.NodeMask = 0;

	// �萔�o�b�t�@�p�f�B�X�N���v�^�q�[�v����
	hr = m_Device->CreateDescriptorHeap(&CBDesc, IID_PPV_ARGS(m_HeapCBV.GetAddressOf()));
	if (FAILED(hr)) return false;

	// �萔�o�b�t�@�ݒ�
	D3D12_HEAP_PROPERTIES CBProp = {};
	CBProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	CBProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	CBProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	CBProp.CreationNodeMask = 1;
	CBProp.VisibleNodeMask = 1;

	// ���\�[�X�̐ݒ�.
	D3D12_RESOURCE_DESC CBResDesc = {};
	CBResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	CBResDesc.Alignment = 0;
	CBResDesc.Width = sizeof(Transform);
	CBResDesc.Height = 1;
	CBResDesc.DepthOrArraySize = 1;
	CBResDesc.MipLevels = 1;
	CBResDesc.Format = DXGI_FORMAT_UNKNOWN;
	CBResDesc.SampleDesc.Count = 1;
	CBResDesc.SampleDesc.Quality = 0;
	CBResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	CBResDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// �C���N�������g�p�T�C�Y�擾
	auto incrementSize = m_Device
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (auto Idx = 0; Idx < FrameCount; ++Idx)
	{
		hr = m_Device->CreateCommittedResource(&CBProp, D3D12_HEAP_FLAG_NONE,
			&CBResDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_ConstantBuffer[Idx].ReleaseAndGetAddressOf()));
		if (FAILED(hr)) return false;

		auto Address = m_ConstantBuffer[Idx]->GetGPUVirtualAddress();
		auto HandleCPU = m_HeapCBV->GetCPUDescriptorHandleForHeapStart();
		auto HandleGPU = m_HeapCBV->GetGPUDescriptorHandleForHeapStart();

		HandleCPU.ptr += incrementSize * Idx;
		HandleGPU.ptr += incrementSize * Idx;

		// �萔�o�b�t�@�r���[�ݒ�
		m_CBView[Idx].HandleCPU = HandleCPU;
		m_CBView[Idx].HandleGPU = HandleGPU;
		m_CBView[Idx].Desc.BufferLocation = Address;
		m_CBView[Idx].Desc.SizeInBytes = sizeof(Transform);

		// �萔�o�b�t�@�r���[����
		m_Device->CreateConstantBufferView(&m_CBView[Idx].Desc, HandleCPU);

		// �萔�f�[�^�]���J�n
		hr = m_ConstantBuffer[Idx]->Map(0, nullptr, reinterpret_cast<void**>(&m_CBView[Idx].pBuffer));
		if (FAILED(hr))
		{
			return false;
		}

		auto eyePos = DirectX::XMVectorSet(0.0f, 0.0f, 5.0f, 0.0f);
		auto targetPos = DirectX::XMVectorZero();
		auto upward = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		auto fovY = DirectX::XMConvertToRadians(37.5f);
		auto aspect = static_cast<float>(1200) / static_cast<float>(800);

		// �ϊ��s��̐ݒ�.
		m_CBView[Idx].pBuffer->World = DirectX::XMMatrixIdentity();
		m_CBView[Idx].pBuffer->View = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
		m_CBView[Idx].pBuffer->Proj = DirectX::XMMatrixPerspectiveFovRH(fovY, aspect, 1.0f, 1000.0f);
	}

	auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	// ���[�g�p�����[�^�ݒ�
	D3D12_ROOT_PARAMETER RootParam = {};
	RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	RootParam.Descriptor.ShaderRegister = 0;
	RootParam.Descriptor.RegisterSpace = 0;
	RootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// ���[�g�V�O�j�`���ݒ�
	D3D12_ROOT_SIGNATURE_DESC RootDesc = {};
	RootDesc.NumParameters = 1;
	RootDesc.NumStaticSamplers = 0;
	RootDesc.pParameters = &RootParam;
	RootDesc.pStaticSamplers = nullptr;
	RootDesc.Flags = flag;

	ComPtr<ID3DBlob> pBlob;
	ComPtr<ID3DBlob> pErrorBlob;

	// �V���A���C�Y
	hr = D3D12SerializeRootSignature(&RootDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
		pBlob.GetAddressOf(), pErrorBlob.ReleaseAndGetAddressOf());
	if (FAILED(hr)) return false;


	// ���[�g�V�O�j�`���𐶐�
	hr = m_Device->CreateRootSignature(0, pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(), IID_PPV_ARGS(m_RootSignature.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) return false;

	D3D12_INPUT_ELEMENT_DESC VertexLayout[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{ "COLOR"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{"NORMAL" , 0,DXGI_FORMAT_R32G32B32_FLOAT,   0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TEXCOORD" , 0, DXGI_FORMAT_R32G32_FLOAT,		 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	// ���X�^���C�U�[�X�e�[�g�ݒ�
	D3D12_RASTERIZER_DESC RasDesc;
	RasDesc.FillMode = D3D12_FILL_MODE_SOLID;
	RasDesc.CullMode = D3D12_CULL_MODE_NONE;
	RasDesc.FrontCounterClockwise = FALSE;
	RasDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	RasDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	RasDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	RasDesc.DepthClipEnable = FALSE;
	RasDesc.MultisampleEnable = FALSE;
	RasDesc.AntialiasedLineEnable = FALSE;
	RasDesc.ForcedSampleCount = 0;
	RasDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	// �����_�[�^�[�Q�b�g�̃u�����h�ݒ�.
	D3D12_RENDER_TARGET_BLEND_DESC RTBDesc = {
		FALSE, FALSE,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL
	};

	// �u�����h�X�e�[�g�̐ݒ�.
	D3D12_BLEND_DESC BSDesc;
	BSDesc.AlphaToCoverageEnable = FALSE;
	BSDesc.IndependentBlendEnable = FALSE;
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
	{
		BSDesc.RenderTarget[i] = RTBDesc;
	}

	// �[�x�X�e���V���X�e�[�g�ݒ�
	D3D12_DEPTH_STENCIL_DESC DepStencilDesc = {};
	DepStencilDesc.DepthEnable = false;
	DepStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	DepStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	DepStencilDesc.StencilEnable = false;

	ComPtr<ID3DBlob> VSBlob;
	ComPtr<ID3DBlob> PSBlob;

	// ���_�V�F�[�_�[�ǂݍ���
	if (!CompileShader("Shader/SimpleVS.hlsl", "main", "vs_5_0", VSBlob.ReleaseAndGetAddressOf()))
	{
		return false;
	}

	// �s�N�Z���V�F�[�_�[�ǂݍ���
	if (!CompileShader("Shader/SimplePS.hlsl", "main", "ps_5_0", PSBlob.ReleaseAndGetAddressOf()))
	{
		return false;
	}

	// �p�C�v���C���X�e�[�g�ݒ�
	D3D12_GRAPHICS_PIPELINE_STATE_DESC PipeStateDesc = {};
	PipeStateDesc.InputLayout = { VertexLayout, _countof(VertexLayout) };
	PipeStateDesc.pRootSignature = m_RootSignature.Get();
	PipeStateDesc.VS.BytecodeLength = VSBlob->GetBufferSize();
	PipeStateDesc.VS.pShaderBytecode = VSBlob->GetBufferPointer();
	PipeStateDesc.PS.BytecodeLength = PSBlob->GetBufferSize();
	PipeStateDesc.PS.pShaderBytecode = PSBlob->GetBufferPointer();
	PipeStateDesc.RasterizerState = RasDesc;
	PipeStateDesc.BlendState = BSDesc;
	PipeStateDesc.DepthStencilState = DepStencilDesc;
	PipeStateDesc.SampleMask = UINT_MAX;
	PipeStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	PipeStateDesc.NumRenderTargets = 1;
	PipeStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	PipeStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	PipeStateDesc.SampleDesc.Count = 1;
	PipeStateDesc.SampleDesc.Quality = 0;

	// �p�C�v���C���X�e�[�g����
	hr = m_Device->CreateGraphicsPipelineState(&PipeStateDesc, IID_PPV_ARGS(m_PSO.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) return false;

	return true;
}