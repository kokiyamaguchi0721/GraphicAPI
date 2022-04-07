#pragma once
#include <iostream>
#include "AllInWrapper.h"
#include "DirectX11Wrapper.h"
#include "DirectX12Wrapper.h"

// アプリケーション管理クラス
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

