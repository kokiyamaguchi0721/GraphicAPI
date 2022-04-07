#include "DirectX12Wrapper.h"

HRESULT DirectX12Wrapper::Create(HWND hwnd, RECT rc)
{
    UINT FlagsDXGI = 0;
    ID3D12Debug* Debug = nullptr;
    HRESULT hr;

#ifdef _DEBUG
    // �f�o�b�O�C���^�[�t�F�[�X
    D3D12GetDebugInterface(IID_PPV_ARGS(&Debug));
    if (Debug)
    {
        Debug->EnableDebugLayer();
        Debug->Release();
    }
    FlagsDXGI |= DXGI_CREATE_FACTORY_DEBUG;
#endif

    // �t�@�N�g���[����
    hr = CreateDXGIFactory2(FlagsDXGI, IID_PPV_ARGS(m_Factory.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) return hr;


    // �A�_�v�^�[�擾
    ComPtr<IDXGIAdapter> Adapter;
    hr = m_Factory->EnumAdapters(0,Adapter.GetAddressOf());
    if (FAILED(hr)) return hr;


    // �f�o�C�X����
    hr = D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_Device.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) return hr;


    // �R�}���h�A���P�[�^����
    hr = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocator.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) return hr;


    // �R�}���h�L���[�ݒ�
    D3D12_COMMAND_QUEUE_DESC CommandQueueDesc;
    ZeroMemory(&CommandQueueDesc,sizeof(CommandQueueDesc));
    CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    CommandQueueDesc.Priority = 0;
    CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    // �R�}���h�L���[����
    hr = m_Device->CreateCommandQueue(&CommandQueueDesc,IID_PPV_ARGS(m_CommandQueue.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) return hr;


    // �t�F���X����
	m_FenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    hr = m_Device->CreateFence(0,D3D12_FENCE_FLAG_NONE,IID_PPV_ARGS(m_Fence.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) return hr;


    // �X���b�v�`�F�C���ݒ�
    DXGI_SWAP_CHAIN_DESC SwapChinDesc;
    ZeroMemory(&SwapChinDesc, sizeof(SwapChinDesc));
    SwapChinDesc.BufferCount = 2;
    SwapChinDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChinDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChinDesc.OutputWindow = hwnd;
    SwapChinDesc.SampleDesc.Count = 1;
    SwapChinDesc.Windowed = true;
    SwapChinDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    SwapChinDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    // �X���b�v�`�F�C������
    hr = m_Factory->CreateSwapChain(m_CommandQueue.Get(), &SwapChinDesc, (IDXGISwapChain**)m_SwapChain.ReleaseAndGetAddressOf());
    if (FAILED(hr)) return hr;


    // �R�}���h���X�g����
    hr = m_Device->CreateCommandList(0,D3D12_COMMAND_LIST_TYPE_DIRECT,m_CommandAllocator.Get(),nullptr,IID_PPV_ARGS(m_CommandList.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) return hr;


    // RTV�p�f�B�X�N���v�^�q�[�v�ݒ�
    D3D12_DESCRIPTOR_HEAP_DESC RTVDecriptorHeapDesc;
    ZeroMemory(&RTVDecriptorHeapDesc, sizeof(RTVDecriptorHeapDesc));
    RTVDecriptorHeapDesc.NumDescriptors = 2;
    RTVDecriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    RTVDecriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    
    // RTV�p�f�B�X�N���v�^�q�[�v����
    m_Device->CreateDescriptorHeap(&RTVDecriptorHeapDesc,IID_PPV_ARGS(m_DescriptorHeap.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) return hr;


    // �����_�[�^�[�Q�b�g����
    UINT StrideHandleBytes = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    for (UINT Idx = 0; Idx < SwapChinDesc.BufferCount; ++Idx)
    {
        hr = m_SwapChain->GetBuffer(Idx,IID_PPV_ARGS(m_RenderTarget[Idx].ReleaseAndGetAddressOf()));
        if (FAILED(hr)) return hr;

        m_RtvHandle[Idx] = m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        m_RtvHandle[Idx].ptr += Idx * StrideHandleBytes;
        m_Device->CreateRenderTargetView(m_RenderTarget[Idx].Get(), nullptr, m_RtvHandle[Idx]);
    }


    // �r���[�|�[�g�ݒ�
    ViewPort.TopLeftX = 0;
    ViewPort.TopLeftY = 0;
    ViewPort.Width = (FLOAT)rc.right;
    ViewPort.Height = (FLOAT)rc.bottom;
    ViewPort.MinDepth = 0;
    ViewPort.MaxDepth = 1;

    return S_OK;
}

void DirectX12Wrapper::Release()
{
}

// �`��O����
void DirectX12Wrapper::BeforeRender()
{
    SetResouceBarrier(m_CommandList.Get(),m_RenderTarget[TargetIndex].Get(),D3D12_RESOURCE_STATE_PRESENT,D3D12_RESOURCE_STATE_RENDER_TARGET);

    // �����_�[�^�[�Q�b�g������
    float ClearColor[4] = { 1.0f,0.0f,0.0f,1.0f };
    TargetIndex = m_SwapChain->GetCurrentBackBufferIndex();
    m_CommandList->ClearRenderTargetView(m_RtvHandle[TargetIndex],ClearColor,0,nullptr);

    // �r���[�|�[�g�Z�b�g
    m_CommandList->RSSetViewports(1, &ViewPort);
}

// �`��㏈��
void DirectX12Wrapper::AfterRender()
{
    SetResouceBarrier(m_CommandList.Get(), m_RenderTarget[TargetIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_CommandList->Close();

    // �R�}���h���s
    ID3D12CommandList* pCommandList = m_CommandList.Get();
    m_CommandQueue->ExecuteCommandLists(1, &pCommandList);
    m_SwapChain->Present(1,0);

    // �t�F���X����
    WaitForCommandQueue(m_CommandQueue.Get());

    // �R�}���h�֘A������
    m_CommandAllocator->Reset();
    m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);
}

// ���\�[�X�o���A
void DirectX12Wrapper::SetResouceBarrier(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* Resouce, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After)
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
    m_CommandList->ResourceBarrier(1,&BarrierDesc);
}

// �t�F���X����
void DirectX12Wrapper::WaitForCommandQueue(ID3D12CommandQueue* CommandQueue)
{
    static UINT64  Frame = 0;
    m_Fence->SetEventOnCompletion(Frame, m_FenceEvent);

    // GPU�̏�Ԃ��擾
    CommandQueue->Signal(m_Fence.Get(), Frame);

    // �ҋ@����
    WaitForSingleObject(m_FenceEvent,INFINITE);
    Frame++;
}
