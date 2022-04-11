#include "App.h"
#include <cassert>


namespace  {
    const auto ClassName = TEXT("SampleWindowClass");    //!< �E�B���h�E�N���X���ł�.

    struct Vertexes
    {
        DirectX::XMFLOAT3   Position;    // �ʒu���W�ł�.
        DirectX::XMFLOAT4   Color;       // ���_�J���[�ł�.
    };

} 


//      �R���X�g���N�^�ł�.
App::App(uint32_t width, uint32_t height)
   /* : m_hInst(nullptr)
    , m_hWnd(nullptr)
    , m_Width(width)
    , m_Height(height)
    , m_FrameIndex(0)*/
{
    //for (auto i = 0u; i < FrameCount; ++i)
    //{
    //    m_pColorBuffer[i] = nullptr;
    //    m_pCmdAllocator[i] = nullptr;
    //    m_FenceCounter[i] = 0;
    //}
}

// �f�X�g���N�^�ł�.
App::~App()
{
}

// ���s���܂�.
void App::Run()
{
    if (InitApp())
    {
        MainLoop();
    }

    TermApp();
}

//      �����������ł�.
bool App::InitApp()
{
    // �E�B���h�E�̏�����.
    if (!InitWnd())
    {
        return false;
    }

    // ����I��.
    return true;
}

//      �I�������ł�.
void App::TermApp()
{
    // �E�B���h�E�̏I������.
    TermWnd();
}

//      �E�B���h�E�̏����������ł�.
bool App::InitWnd()
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
    wc.lpszClassName = ClassName;
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
        ClassName,
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

//      �E�B���h�E�̏I�������ł�.
void App::TermWnd()
{
    // �E�B���h�E�̓o�^������.
    if (m_hInst != nullptr)
    {
        UnregisterClass(ClassName, m_hInst);
    }

    m_hInst = nullptr;
    m_hWnd = nullptr;
}

//      ���C�����[�v�ł�.
void App::MainLoop()
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

LRESULT CALLBACK App::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
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
