#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
//#include <d3d12shader.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include "DirectXAllRapper.h"

#define _DEBUG

class DirectX12Wrapper : public DirectXAllRapper
{
private:
	ComPtr<ID3D12Device> m_Device;
	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	ComPtr<IDXGISwapChain3> m_SwapChain;
	ComPtr<ID3D12Fence> m_Fence;
	HANDLE m_FenceEvent;
	ComPtr<IDXGIFactory3> m_Factory;
	ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap;
	ComPtr < ID3D12Resource> m_RenderTarget[2];
	D3D12_CPU_DESCRIPTOR_HANDLE m_RtvHandle[2];
	D3D12_VIEWPORT ViewPort;
	int TargetIndex;

public:
	HRESULT Create(HWND hwnd, RECT rc);
	void    Release();
	void	BeforeRender();
	void	AfterRender();
	void	SetResouceBarrier(ID3D12GraphicsCommandList* CommandList,ID3D12Resource* Resouce,D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After);
	void	WaitForCommandQueue(ID3D12CommandQueue* CommandQueue);
};

