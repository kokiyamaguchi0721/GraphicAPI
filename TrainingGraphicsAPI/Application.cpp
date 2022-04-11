#include "Application.h"

Application::Application(uint32_t Width, uint32_t Height)
{
	m_Width = Width;
	m_Height = Height;
}

Application::~Application()
{
}

bool Application::InitWnd()
{
	// �C���X�^���X�n���h�����擾.
	auto hInst = GetModuleHandle(nullptr);
	if (hInst == nullptr)
	{
		return false;
	}

	// �E�B���h�E�̐ݒ�.
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hIcon = LoadIcon(hInst, IDI_APPLICATION);
	wc.hCursor = LoadCursor(hInst, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = "Sampl";
	wc.hIconSm = LoadIcon(hInst, IDI_APPLICATION);

	// �E�B���h�E�̓o�^.
	if (!RegisterClassEx(&wc))
	{
		return false;
	}

	// �C���X�^���X�n���h���ݒ�.
	m_hInst = hInst;

	// �E�B���h�E�̃T�C�Y��ݒ�.
	RECT rc = {};
	rc.right = static_cast<LONG>(m_Width);
	rc.bottom = static_cast<LONG>(m_Height);

	// �E�B���h�E�T�C�Y�𒲐�.
	auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&rc, style, FALSE);

	// �E�B���h�E�𐶐�.
	m_hWnd = CreateWindowEx(
		0,
		"Sample",
		TEXT("Sample"),
		style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		m_hInst,
		nullptr);

	if (m_hWnd == nullptr)
	{
		return false;
	}

	// �E�B���h�E��\��.
	ShowWindow(m_hWnd, SW_SHOWNORMAL);

	// �E�B���h�E���X�V.
	UpdateWindow(m_hWnd);

	// �E�B���h�E�Ƀt�H�[�J�X��ݒ�.
	SetFocus(m_hWnd);

	// ����I��.
	return true;
}

void Application::Run()
{
	//(this->*Process)();

	if (Initialize())
	{
		MainLoop();
	}

	Finalize();
}

void Application::MainLoop()
{
	MSG msg = {};

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) == TRUE)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// Render();
		}
	}
}

bool Application::Initialize()
{
	// �E�B���h�E�̏�����.
	if (!InitWnd())
	{
		return false;
	}

	//std::cout << "DirectX11 >> 0" << std::endl;
	//std::cout << "DirectX12 >> 1" << std::endl;

	//// �ǂ�API���g�p���邩�I��
	//int ChoiceIdx;
	//while (true)
	//{
	//	std::cin >> ChoiceIdx;
	//	if (ChoiceIdx <= 1)
	//	{
	//		break;
	//	}
	//}

	//switch (ChoiceIdx)
	//{
	//	
	//case 0:												// DirectX11 
	//	//ApiWrapper = new DirectX11Wrapper;				// API�^�C�v�Z�b�g
	//	//Process = &Application::RunWindowsAPI;			// �֐��|�C���^�Z�b�g
	//	break;

	//	
	//case 1:												// DirectX12
	//	//ApiWrapper = new DirectX12Wrapper;				// API�^�C�v�Z�b�g
	//	//Process = &Application::RunWindowsAPI;			// �֐��|�C���^�Z�b�g
	//	break;

	//default:
	//	return false;
	//	break;
	//}

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

LRESULT CALLBACK Application::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_DESTROY:
	{ PostQuitMessage(0); }
	break;

	default:
	{ /* DO_NOTHING */ }
	break;
	}

	return DefWindowProc(hWnd, msg, wp, lp);
}
