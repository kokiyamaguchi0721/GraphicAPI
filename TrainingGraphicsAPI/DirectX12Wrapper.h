#pragma once
#include <d3d12.h>

#include "DirectXAllRapper.h"

class DirectX12Wrapper : public DirectXAllRapper
{
protected:
private:
public:
	HRESULT Create(HWND hwnd, RECT rc);
	void    Release();
	void	BeforeRender();
	void	AfterRender();
};

