#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <vector>
#include <string>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include "DirectXAllRapper.h"

#define _DEBUG

class DirectX12Wrapper : public DirectXAllRapper
{
private:
	ComPtr<ID3D12Device>						m_Device;
	ComPtr<ID3D12CommandQueue>					m_CommandQueue;
	ComPtr<ID3D12CommandAllocator>				m_CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList>			m_CommandList;
	ComPtr<IDXGISwapChain3>						m_SwapChain;
	ComPtr<ID3D12Fence>							m_Fence;
	UINT64										FenceVal	= 0;
	ComPtr<IDXGIFactory3>						m_Factory;
	ComPtr<ID3D12DescriptorHeap>				RTVHeaps;
	D3D12_VIEWPORT								ViewPort;
	std::vector<ComPtr<ID3D12Resource>>			BackBuffers;
	D3D12_RECT									ScissorRect;
	ComPtr<ID3D12Resource>						DepthBuffer = nullptr;
	ComPtr<ID3D12DescriptorHeap>				dsvHeap		= nullptr;
	D3D12_DEPTH_STENCIL_VIEW_DESC				dsvDesc;
	UINT										BuckBuffIdx;


	ComPtr<ID3D12RootSignature>					m_RootSignature;
	ComPtr<ID3D12PipelineState>					m_PipelineState;
	ComPtr<ID3D12Resource>						m_VertexBuffer;
	ComPtr<ID3D12Resource>						m_IndexBuffer;
	ComPtr<ID3D12Resource>						m_ConstBuffer;
	D3D12_VERTEX_BUFFER_VIEW					m_VBView;
	D3D12_INDEX_BUFFER_VIEW						m_IBView;
	ComPtr<ID3DBlob>							m_VertexShader;
	ComPtr<ID3DBlob>							m_PixelShader;
	ComPtr<ID3D12DescriptorHeap>				m_BasicDescHeap = nullptr;
	
public:
	HRESULT Create(HWND hwnd, RECT rc);
	void    Release();
	void	BeforeRender();
	void	AfterRender();
	void	SetResouceBarrier(ID3D12Resource* Resouce, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After);
	void	WaitForCommandQueue();

	bool	PolygonInit();
	bool	CubeInit();
	void	CubeUpdate();
	void	ObjectDraw();
	bool	CreateTexture();
};

