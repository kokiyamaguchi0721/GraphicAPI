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
	// インスタンスハンドルを取得.
	auto hInst = GetModuleHandle(nullptr);
	if (hInst == nullptr)
	{
		return false;
	}

	// ウィンドウの設定.
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

	// ウィンドウの登録.
	if (!RegisterClassEx(&wc))
	{
		return false;
	}

	// インスタンスハンドル設定.
	m_hInst = hInst;

	// ウィンドウのサイズを設定.
	RECT rc = {};
	rc.right = static_cast<LONG>(m_Width);
	rc.bottom = static_cast<LONG>(m_Height);

	// ウィンドウサイズを調整.
	auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&rc, style, FALSE);

	// ウィンドウを生成.
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

	// ウィンドウを表示.
	ShowWindow(m_hWnd, SW_SHOWNORMAL);

	// ウィンドウを更新.
	UpdateWindow(m_hWnd);

	// ウィンドウにフォーカスを設定.
	SetFocus(m_hWnd);

	// 正常終了.
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
	// ウィンドウの初期化.
	if (!InitWnd())
	{
		return false;
	}

	//std::cout << "DirectX11 >> 0" << std::endl;
	//std::cout << "DirectX12 >> 1" << std::endl;

	//// どのAPIを使用するか選択
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
	//	//ApiWrapper = new DirectX11Wrapper;				// APIタイプセット
	//	//Process = &Application::RunWindowsAPI;			// 関数ポインタセット
	//	break;

	//	
	//case 1:												// DirectX12
	//	//ApiWrapper = new DirectX12Wrapper;				// APIタイプセット
	//	//Process = &Application::RunWindowsAPI;			// 関数ポインタセット
	//	break;

	//default:
	//	return false;
	//	break;
	//}

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
