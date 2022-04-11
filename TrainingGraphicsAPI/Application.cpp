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
	// インスタンスハンドルを取得.
	auto hInst = GetModuleHandle(nullptr);
	if (hInst == nullptr)
	{
		return false;
	}

	DWORD timeBefore;

	// メモリリークを検知
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	WNDCLASSEX		wcex;								// ウインドウクラス構造体

	// ウインドウクラス情報のセット
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

	// ウインドウクラスの登録
	if (!RegisterClassEx(&wcex)) return FALSE;

	// インスタンスハンドル設定.
	m_hInst = hInst;

	// ウィンドウのサイズを設定.
	RECT rc = {};
	rc.right = static_cast<LONG>(m_Width);
	rc.bottom = static_cast<LONG>(m_Height);

	// ウィンドウサイズを調整.
	auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&rc, style, FALSE);

	m_hWnd = CreateWindow(NAME, TITLE, WS_CAPTION | WS_SYSMENU, 0,
		0, rc.right - rc.left, rc.bottom - rc.top, HWND_DESKTOP, (HMENU)NULL, m_hInst, (LPVOID)NULL);

	if (!m_hWnd) return false;

	timeBefore = (DWORD)GetTickCount64();

	// タイマーセット
	SetTimer(m_hWnd, TIMER_ID, FREAM_RATE, NULL);


	ShowWindow(m_hWnd, SW_SHOWNORMAL);
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

	// メッセージループ
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
		ApiWrapper = new DirectX11Wrapper;				// APIタイプセット
		//Process = &Application::RunWindowsAPI;			// 関数ポインタセット
		break;


	case 1:												// DirectX12
		ApiWrapper = new DirectX12Wrapper;				// APIタイプセット
		//Process = &Application::RunWindowsAPI;			// 関数ポインタセット
		break;

	default:
		return false;
		break;
	}

	// ウィンドウの初期化.
	if (!InitWnd())
	{
		return false;
	}

	RECT rc = { 0,0,m_Width,m_Height };

	//API初期化
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
