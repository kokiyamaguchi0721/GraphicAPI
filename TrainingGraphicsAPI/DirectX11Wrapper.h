#pragma once
#include <d3d11.h>
#include <string>
#include <random>

#include "DirectXAllRapper.h"

#pragma comment(lib,"d3d11.lib")

class DirectX11Wrapper :public DirectXAllRapper
{
private:
	ComPtr<ID3D11Device>				m_Device;
	ComPtr<ID3D11DeviceContext>			m_ImmediateContext;
	ComPtr<IDXGISwapChain>				m_SwapChain;
	ComPtr<ID3D11RenderTargetView>		m_RenderTargetView;
	ComPtr<ID3D11Texture2D>				m_DepthStencilTexture;
	ComPtr<ID3D11DepthStencilView>		m_DepthStencilView;
	ComPtr<ID3D11Buffer>				m_VertexBuffer;
	ComPtr<ID3D11Buffer>				m_IndexBuffer;
	ComPtr<ID3D11Buffer>				m_ConstantBuffer;
	ComPtr<ID3D11InputLayout>			m_InputLayOut;
	ComPtr<ID3D11VertexShader>			m_VertexShader;
	ComPtr<ID3D11PixelShader>			m_PixelShader;
	D3D11_VIEWPORT						m_ViewPort = {};
	ComPtr<ID3D11ShaderResourceView>	NoiseTextureResouce;
	ComPtr<ID3D11SamplerState>			m_SamplerState;
	int									IndexNum=0;

public:
	HRESULT Create(HWND hwnd, RECT rc) override final;
	void    Release() override final;
	void	BeforeRender() override final;
	void	AfterRender() override final;
	bool	PolygonInit() override final;
	bool	CubeInit() override final;
	void	ObjectDraw() override final;
	void	ObjectUpdate();
	bool	CreateTexture();
};