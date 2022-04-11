#include "main.h"
//
//#if defined(DEBUG) || defined(_DEBUG)
//#define _CRTDBG_MAP_ALLOC
//#include <crtdbg.h>
//#endif
//
//#include "Application.h"
//
//int wmain(int argc, wchar_t** argv, wchar_t** evnp)
//{
//#if defined(DEBUG) || defined(_DEBUG)
//    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//#endif
//
//    // �A�v���P�[�V�������s
//    Application app(SCREEN_WIDTH, SCREEN_HEIGHT);
//    app.Run();
//
//    return 0;
//}


#define TIMER_ID 1
#define FREAM_RATE (1000 / 60)

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	//DirectX11Wrapper Directx;
	DirectX12Wrapper Directx;

	HWND			hwnd;								// �E�C���h�E�n���h��
	MSG				msg;								// ���b�Z�[�W�\����
	WNDCLASSEX		wcex;								// �E�C���h�E�N���X�\����
	DWORD timeBefore;

	// ���������[�N�����m
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// �E�C���h�E�N���X���̃Z�b�g
	wcex.hInstance = hInstance;
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


	RECT rc = { 0,0,SCREEN_WIDTH,SCREEN_HEIGHT };
	hwnd = CreateWindow(NAME, TITLE, WS_CAPTION | WS_SYSMENU, 0,
		0, rc.right - rc.left, rc.bottom - rc.top, HWND_DESKTOP, (HMENU)NULL, hInstance, (LPVOID)NULL);

	if (!hwnd) return FALSE;

	timeBefore = (DWORD)GetTickCount64();

	// �^�C�}�[�Z�b�g
	SetTimer(hwnd, TIMER_ID, FREAM_RATE, NULL);

	// DirectxX12�̏�����
	if (FAILED(Directx.Create(hwnd, rc)))
	{
		return 0;
	}

	//Directx.PolygonInit();
	Directx.CubeInit();

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

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

				// DirectX �`��O����
				Directx.BeforeRender();

				//Directx.CubeUpdate();

				// �I�u�W�F�N�g�`��
				Directx.ObjectDraw();
				//Directx.Render();

				// DirectX �`��㏈��
				Directx.AfterRender();
			}
		}
		else
		{
			Sleep(5);
		}
	}

	// DirectX1�I������
	Directx.Release();

	// �^�C�}�[�J��
	KillTimer(hwnd, TIMER_ID);

	return (int)msg.wParam;
}

// �E�B���h�E�v���V�[�W��
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}