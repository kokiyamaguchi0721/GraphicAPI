#include "DirectX12Wrapper.h"

HRESULT DirectX12Wrapper::Create(HWND hwnd, RECT rc)
{
    UINT FlagsDXGI = 0;
    ID3D12Debug* Debug = nullptr;
    HRESULT hr;

#ifdef _DEBUG
    // デバッグインターフェース
    D3D12GetDebugInterface(IID_PPV_ARGS(&Debug));
    if (Debug)
    {
        Debug->EnableDebugLayer();
        Debug->Release();
    }
    FlagsDXGI |= DXGI_CREATE_FACTORY_DEBUG;
#endif

    // ファクトリー生成
    hr = CreateDXGIFactory2(FlagsDXGI, IID_PPV_ARGS(m_Factory.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) return hr;


    // アダプター取得
    ComPtr<IDXGIAdapter> Adapter;
    hr = m_Factory->EnumAdapters(0,Adapter.GetAddressOf());
    if (FAILED(hr)) return hr;


    // デバイス生成
    hr = D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_Device.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) return hr;


    // コマンドアロケータ生成
    hr = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocator.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) return hr;


    // コマンドキュー設定
    D3D12_COMMAND_QUEUE_DESC CommandQueueDesc;
    ZeroMemory(&CommandQueueDesc,sizeof(CommandQueueDesc));
    CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    CommandQueueDesc.Priority = 0;
    CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    // コマンドキュー生成
    hr = m_Device->CreateCommandQueue(&CommandQueueDesc,IID_PPV_ARGS(m_CommandQueue.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) return hr;


    // フェンス生成
	m_FenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    hr = m_Device->CreateFence(0,D3D12_FENCE_FLAG_NONE,IID_PPV_ARGS(m_Fence.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) return hr;


    // スワップチェイン設定
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

    // スワップチェイン生成
    hr = m_Factory->CreateSwapChain(m_CommandQueue.Get(), &SwapChinDesc, (IDXGISwapChain**)m_SwapChain.ReleaseAndGetAddressOf());
    if (FAILED(hr)) return hr;


    // コマンドリスト生成
    hr = m_Device->CreateCommandList(0,D3D12_COMMAND_LIST_TYPE_DIRECT,m_CommandAllocator.Get(),nullptr,IID_PPV_ARGS(m_CommandList.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) return hr;


    // RTV用ディスクリプタヒープ設定
    D3D12_DESCRIPTOR_HEAP_DESC RTVDecriptorHeapDesc;
    ZeroMemory(&RTVDecriptorHeapDesc, sizeof(RTVDecriptorHeapDesc));
    RTVDecriptorHeapDesc.NumDescriptors = 2;
    RTVDecriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    RTVDecriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    
    // RTV用ディスクリプタヒープ生成
    m_Device->CreateDescriptorHeap(&RTVDecriptorHeapDesc,IID_PPV_ARGS(m_DescriptorHeap.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) return hr;


    // レンダーターゲット生成
    UINT StrideHandleBytes = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    for (UINT Idx = 0; Idx < SwapChinDesc.BufferCount; ++Idx)
    {
        hr = m_SwapChain->GetBuffer(Idx,IID_PPV_ARGS(m_RenderTarget[Idx].ReleaseAndGetAddressOf()));
        if (FAILED(hr)) return hr;

        m_RtvHandle[Idx] = m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        m_RtvHandle[Idx].ptr += Idx * StrideHandleBytes;
        m_Device->CreateRenderTargetView(m_RenderTarget[Idx].Get(), nullptr, m_RtvHandle[Idx]);
    }


    // ビューポート設定
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

// 描画前処理
void DirectX12Wrapper::BeforeRender()
{
    SetResouceBarrier(m_CommandList.Get(),m_RenderTarget[TargetIndex].Get(),D3D12_RESOURCE_STATE_PRESENT,D3D12_RESOURCE_STATE_RENDER_TARGET);

    // レンダーターゲット初期化
    float ClearColor[4] = { 1.0f,0.0f,0.0f,1.0f };
    TargetIndex = m_SwapChain->GetCurrentBackBufferIndex();
    m_CommandList->ClearRenderTargetView(m_RtvHandle[TargetIndex],ClearColor,0,nullptr);

    // ビューポートセット
    m_CommandList->RSSetViewports(1, &ViewPort);
}

// 描画後処理
void DirectX12Wrapper::AfterRender()
{
    SetResouceBarrier(m_CommandList.Get(), m_RenderTarget[TargetIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_CommandList->Close();

    // コマンド実行
    ID3D12CommandList* pCommandList = m_CommandList.Get();
    m_CommandQueue->ExecuteCommandLists(1, &pCommandList);
    m_SwapChain->Present(1,0);

    // フェンス処理
    WaitForCommandQueue(m_CommandQueue.Get());

    // コマンド関連初期化
    m_CommandAllocator->Reset();
    m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);
}

// リソースバリア
void DirectX12Wrapper::SetResouceBarrier(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* Resouce, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After)
{
    // リソースバリア設定
    D3D12_RESOURCE_BARRIER BarrierDesc;
    ZeroMemory(&BarrierDesc, sizeof(BarrierDesc));
    BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    BarrierDesc.Transition.pResource = Resouce;
    BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    BarrierDesc.Transition.StateBefore = Before;
    BarrierDesc.Transition.StateAfter = After;

    // リソースバリアセット
    m_CommandList->ResourceBarrier(1,&BarrierDesc);
}

// フェンス処理
void DirectX12Wrapper::WaitForCommandQueue(ID3D12CommandQueue* CommandQueue)
{
    static UINT64  Frame = 0;
    m_Fence->SetEventOnCompletion(Frame, m_FenceEvent);

    // GPUの状態を取得
    CommandQueue->Signal(m_Fence.Get(), Frame);

    // 待機処理
    WaitForSingleObject(m_FenceEvent,INFINITE);
    Frame++;
}
