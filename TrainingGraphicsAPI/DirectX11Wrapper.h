#pragma once
#include <d3d11.h>
#include <string>
#include <random>

#include "DirectXAllRapper.h"

#pragma comment(lib,"d3d11.lib")

class DirectX11Wrapper :public DirectXAllRapper
{
private:
	ComPtr<ID3D11Device> m_Device;
	ComPtr<ID3D11DeviceContext> m_ImmediateContext;
	ComPtr<IDXGISwapChain> m_SwapChain;
	ComPtr<ID3D11RenderTargetView> m_RenderTargetView;
	ComPtr<ID3D11Texture2D> m_DepthStencilTexture;
	ComPtr<ID3D11DepthStencilView> m_DepthStencilView;
	ComPtr<ID3D11Buffer> m_VertexBuffer;
	ComPtr<ID3D11Buffer> m_IndexBuffer;
	ComPtr<ID3D11Buffer> m_ConstantBuffer;
	ComPtr<ID3D11InputLayout> m_InputLayOut;
	ComPtr<ID3D11VertexShader> m_VertexShader;
	ComPtr<ID3D11PixelShader> m_PixelShader;
	D3D11_VIEWPORT m_ViewPort;
	ComPtr<ID3D11ShaderResourceView> NoiseTextureResouce;
	ComPtr<ID3D11SamplerState> m_SamplerState;



public:
	HRESULT Create(HWND hwnd, RECT rc);
	void    Release();
	void	BeforeRender();
	void	AfterRender();

	bool	PolygonInit();
	void	PolygonDraw();
	bool	CubeInit();
	void	CubeDraw();
	bool	CreateTexture();
};