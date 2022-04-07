#include "main.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	//DirectX11Wrapper Dx11;

	HWND			hwnd;								// �E�C���h�E�n���h��
	MSG				msg;								// ���b�Z�[�W�\����
	WNDCLASSEX		wcex;								// �E�C���h�E�N���X�\����

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


	//// DirectxX11�̏�����
	//if (FAILED(Dx11.Create(hwnd, rc)))
	//{
	//	return 0;
	//}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	// ���b�Z�[�W���[�v
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		//// DirectX11 �`��O����
		//Dx11.BeforeRender();

		//// 2D�|���S���`��
		////Dx11.PolygonDraw();

		//// 3D�L���[�u�`��
		//Dx11.CubeDraw();

		//// DirectX11 �`��㏈��
		//Dx11.AfterRender();
	}

	Dx11.Release();

	return msg.wParam;
}

// �E�B���h�E�v���V�[�W��
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_KEYDOWN:
		switch (wParam) {
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