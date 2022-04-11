#include "Application.h"

#define TIMER_ID 1
#define FREAM_RATE (1000 / 60)

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

	DWORD timeBefore;

	// ���������[�N�����m
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	WNDCLASSEX		wcex;								// �E�C���h�E�N���X�\����

	// �E�C���h�E�N���X���̃Z�b�g
	wcex.hInstance = m_hInst;
	wcex.lpszClassName = NAME;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.style = 0;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hIcon = LoadIcon((HINSTANCE)NULL, IDI_APPLICATION);
	wcex.hIconSm = LoadIcon((HINSTANCE)NULL, IDI_WINLOGO);
	wcex.hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
	wcex.lpszMenuName = 0;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

	// �E�C���h�E�N���X�̓o�^
	if (!RegisterClassEx(&wcex)) return FALSE;

	// �C���X�^���X�n���h���ݒ�.
	m_hInst = hInst;

	// �E�B���h�E�̃T�C�Y��ݒ�.
	RECT rc = {};
	rc.right = static_cast<LONG>(m_Width);
	rc.bottom = static_cast<LONG>(m_Height);

	// �E�B���h�E�T�C�Y�𒲐�.
	auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&rc, style, FALSE);

	m_hWnd = CreateWindow(NAME, TITLE, WS_CAPTION | WS_SYSMENU, 0,
		0, rc.right - rc.left, rc.bottom - rc.top, HWND_DESKTOP, (HMENU)NULL, m_hInst, (LPVOID)NULL);

	if (!m_hWnd) return false;

	timeBefore = (DWORD)GetTickCount64();

	// �^�C�}�[�Z�b�g
	SetTimer(m_hWnd, TIMER_ID, FREAM_RATE, NULL);


	ShowWindow(m_hWnd, SW_SHOWNORMAL);
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

	// ���b�Z�[�W���[�v
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);

				Render();
			}
		}
		else
		{
			Sleep(5);
		}
	}
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
		ApiWrapper = new DirectX11Wrapper;				// API�^�C�v�Z�b�g
		//Process = &Application::RunWindowsAPI;			// �֐��|�C���^�Z�b�g
		break;


	case 1:												// DirectX12
		ApiWrapper = new DirectX12Wrapper;				// API�^�C�v�Z�b�g
		//Process = &Application::RunWindowsAPI;			// �֐��|�C���^�Z�b�g
		break;

	default:
		return false;
		break;
	}

	// �E�B���h�E�̏�����.
	if (!InitWnd())
	{
		return false;
	}

	RECT rc = { 0,0,m_Width,m_Height };

	//API������
	HRESULT hr = ApiWrapper->Create(m_hWnd, rc);
	if (FAILED(hr)) return false;

	//ApiWrapper->PolygonInit();
	ApiWrapper->CubeInit();

	return true;
}

void Application::Update()
{
}

void Application::Render()
{
	if (ApiWrapper != nullptr)
	{
		ApiWrapper->BeforeRender();

		ApiWrapper->ObjectDraw();

		ApiWrapper->AfterRender();
	}
}

void Application::Finalize()
{
	if (ApiWrapper != nullptr)
	{
		ApiWrapper->Release();
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
	{ }
	break;
	}

	return DefWindowProc(hWnd, msg, wp, lp);
}
