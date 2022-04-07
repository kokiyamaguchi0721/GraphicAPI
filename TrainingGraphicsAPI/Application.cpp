#include "Application.h"

void Application::Run()
{
	(this->*Process)();
}

bool Application::Initialize()
{
	std::cout << "DirectX11 >> 0" << std::endl;
	std::cout << "DirectX12 >> 1" << std::endl;

	// どのAPIを使用するか選択
	int ChoiceIdx;
	while (true)
	{
		std::cin >> ChoiceIdx;
		if (ChoiceIdx <= 1)
		{
			break;
		}
	}

	switch (ChoiceIdx)
	{
		
	case 0:												// DirectX11 
		//ApiWrapper = new DirectX11Wrapper;				// APIタイプセット
		Process = &Application::RunWindowsAPI;			// 関数ポインタセット
		break;

		
	case 1:												// DirectX12
		//ApiWrapper = new DirectX12Wrapper;				// APIタイプセット
		Process = &Application::RunWindowsAPI;			// 関数ポインタセット
		break;

	default:
		return false;
		break;
	}

	// API初期化
	//ApiWrapper->Initialize();

	return true;
}

void Application::Update()
{
}

void Application::Render()
{
	if (ApiWrapper != nullptr)
	{
		//ApiWrapper->Render();
	}
}

void Application::Finalize()
{
	if (ApiWrapper != nullptr)
	{
		//ApiWrapper->Finalize();
	}
	delete ApiWrapper;
	ApiWrapper = nullptr;

}

void Application::RunWindowsAPI()
{
	std::cout << "WindowsAPIでウィンドウを更新しています。" << std::endl;
	Render();
	Update();
}
