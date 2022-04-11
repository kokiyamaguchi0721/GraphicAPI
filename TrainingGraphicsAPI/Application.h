#pragma once
#include <iostream>
#include <cstdint>

#include "AllInWrapper.h"
#include "DirectX11Wrapper.h"
#include "DirectX12Wrapper.h"

#define		NAME			"APIPractice"
#define		TITLE			"Renderer"

// �A�v���P�[�V�����Ǘ��N���X
class Application
{
private:
	HINSTANCE   m_hInst;        // �C���X�^���X�n���h���ł�.
	HWND        m_hWnd;         // �E�B���h�E�n���h���ł�.
	uint32_t    m_Width;        // �E�B���h�E�̉����ł�.
	uint32_t    m_Height;       // �E�B���h�E�̏c���ł�.

	void Update();
	void Render();
	DirectXAllRapper* ApiWrapper;
public:
	//void (Application::* Process)();
	bool Initialize();
	void Finalize();

	

	Application(uint32_t Width, uint32_t Height);
	~Application();
	bool InitWnd();
	void Run();
	void MainLoop();
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
};

