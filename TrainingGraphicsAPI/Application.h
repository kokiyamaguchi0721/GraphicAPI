#pragma once
#include <iostream>
#include "AllInWrapper.h"
#include "DirectX11Wrapper.h"
#include "DirectX12Wrapper.h"

// �A�v���P�[�V�����Ǘ��N���X
class Application
{
private:
	void Update();
	void Render();

	AllInWrapper* ApiWrapper;
public:
	void (Application::* Process)();
	void Run();
	bool Initialize();
	void Finalize();
	void RunWindowsAPI();
};

