#include "Application.h"

void Application::Run()
{
	(this->*Process)();
}

bool Application::Initialize()
{
	std::cout << "DirectX11 >> 0" << std::endl;
	std::cout << "DirectX12 >> 1" << std::endl;

	// �ǂ�API���g�p���邩�I��
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
		//ApiWrapper = new DirectX11Wrapper;				// API�^�C�v�Z�b�g
		Process = &Application::RunWindowsAPI;			// �֐��|�C���^�Z�b�g
		break;

		
	case 1:												// DirectX12
		//ApiWrapper = new DirectX12Wrapper;				// API�^�C�v�Z�b�g
		Process = &Application::RunWindowsAPI;			// �֐��|�C���^�Z�b�g
		break;

	default:
		return false;
		break;
	}

	// API������
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
	std::cout << "WindowsAPI�ŃE�B���h�E���X�V���Ă��܂��B" << std::endl;
	Render();
	Update();
}
