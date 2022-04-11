#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <vector>
#include <string>
#include <cstdint>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include "DirectXAllRapper.h"

template<typename T>
struct ConstantBufferView
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC Desc;               // 定数バッファの構成設定です.
	D3D12_CPU_DESCRIPTOR_HANDLE     HandleCPU;          // CPUディスクリプタハンドルです.
	D3D12_GPU_DESCRIPTOR_HANDLE     HandleGPU;          // GPUディスクリプタハンドルです.
	T* pBuffer;            // バッファ先頭へのポインタです.
};

struct alignas(256) Transform
{
	DirectX::XMMATRIX   World;      // ワールド行列です.
	DirectX::XMMATRIX   View;       // ビュー行列です.
	DirectX::XMMATRIX   Proj;       // 射影行列です.
};

struct Texture
{
	ComPtr<ID3D12Resource> pResouce;
	D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE HandleGPU;
};

class DirectX12Wrapper : public DirectXAllRapper
{
private:
	static const uint32_t FrameCount = 2;
	ComPtr<ID3D12Device>                   m_Device;
	ComPtr<ID3D12CommandQueue>             m_CmdQueue;
	ComPtr<IDXGISwapChain3>                m_SwapChain;
	ComPtr<ID3D12Resource>                 m_ColorBuffer[FrameCount];
	ComPtr<ID3D12CommandAllocator>         m_CmdAllocator[FrameCount];
	ComPtr<ID3D12GraphicsCommandList>      m_CmdList;
	ComPtr<ID3D12DescriptorHeap>           m_HeapRTV;
	ComPtr<ID3D12DescriptorHeap>           m_HeapDSV;
	ComPtr<ID3D12Fence>                    m_Fence;
	ComPtr<ID3D12DescriptorHeap>           m_BasicDescHeap;
	ComPtr<ID3D12Resource>                 m_VertexBuffer;
	ComPtr<ID3D12Resource>                 m_IndexBuffer;
	ComPtr<ID3D12Resource>                 m_ConstantBuffer[FrameCount * 2];
	ComPtr<ID3D12RootSignature>            m_RootSignature;
	ComPtr<ID3D12PipelineState>            m_PSO;
	ComPtr<ID3D12Resource>				   m_DepthBuffer;

	HANDLE                          m_FenceEvent;
	uint64_t                        m_FenceCounter[FrameCount];
	uint32_t                        m_FrameIndex;
	D3D12_CPU_DESCRIPTOR_HANDLE     m_HandleRTV[FrameCount];
	D3D12_CPU_DESCRIPTOR_HANDLE     m_HandleDSV;
	D3D12_VERTEX_BUFFER_VIEW        m_VBView;
	D3D12_INDEX_BUFFER_VIEW         m_IBView;
	D3D12_VIEWPORT                  m_ViewPort;
	D3D12_RECT                      m_Scissor;
	ConstantBufferView<Transform>   m_CBView[FrameCount * 2];
	float                           m_RotateAngle;
	int								m_IndexNum;
	Texture							m_Texture;

public:
	HRESULT Create(HWND hwnd, RECT rc) override final;
	void    Release() override final;
	void	BeforeRender() override final;
	void	AfterRender() override final;
	bool	PolygonInit() override final;
	bool	CubeInit() override final;
	void	ObjectDraw() override final;

	void	SetResouceBarrier(ID3D12Resource* Resouce, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After);
	void	WaitGPU();
	bool	CreateTexture();
};

