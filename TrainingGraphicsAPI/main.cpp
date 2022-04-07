#include "main.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	//DirectX11Wrapper Dx11;

	HWND			hwnd;								// ウインドウハンドル
	MSG				msg;								// メッセージ構造体
	WNDCLASSEX		wcex;								// ウインドウクラス構造体

	// メモリリークを検知
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// ウインドウクラス情報のセット
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

	// ウインドウクラスの登録
	if (!RegisterClassEx(&wcex)) return FALSE;


	RECT rc = { 0,0,SCREEN_WIDTH,SCREEN_HEIGHT };
	hwnd = CreateWindow(NAME, TITLE, WS_CAPTION | WS_SYSMENU, 0,
		0, rc.right - rc.left, rc.bottom - rc.top, HWND_DESKTOP, (HMENU)NULL, hInstance, (LPVOID)NULL);

	if (!hwnd) return FALSE;


	//// DirectxX11の初期化
	//if (FAILED(Dx11.Create(hwnd, rc)))
	//{
	//	return 0;
	//}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	// メッセージループ
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		//// DirectX11 描画前処理
		//Dx11.BeforeRender();

		//// 2Dポリゴン描画
		////Dx11.PolygonDraw();

		//// 3Dキューブ描画
		//Dx11.CubeDraw();

		//// DirectX11 描画後処理
		//Dx11.AfterRender();
	}

	Dx11.Release();

	return msg.wParam;
}

// ウィンドウプロシージャ
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