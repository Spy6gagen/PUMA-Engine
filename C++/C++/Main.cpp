#include <windows.h>
#include "Engine.h"

LPCWSTR szTitle = L"PUMA ENGINE";
LPCWSTR szWindowClass = L"DIRECTXGAMEWINDOW";
HINSTANCE hInst;
Engine* g_engine = nullptr;
HWND hBtnLevelDesigner = nullptr;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = szWindowClass;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    return RegisterClass(&wc);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;
    HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd)
        return FALSE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Create the Level Designer button
    hBtnLevelDesigner = CreateWindow(
        L"BUTTON", L"Open Level Designer",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10, 10, 180, 30,
        hWnd, (HMENU)1001, hInstance, NULL);

    // Create and initialize the engine
    g_engine = new Engine(hWnd);
    if (!g_engine->Initialize()) {
        MessageBox(hWnd, L"Engine initialization failed!", L"Error", MB_OK);
        delete g_engine;
        g_engine = nullptr;
        return FALSE;
    }

    SetTimer(hWnd, 1, 16, NULL); // ~60fps
    return TRUE;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    MyRegisterClass(hInstance);
    if (!InitInstance(hInstance, nCmdShow))
        return FALSE;

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case 1001: // Level Designer button clicked
            MessageBox(hWnd, L"Level Designer button clicked!", L"Info", MB_OK);
            // Replace this with your actual level designer launch function
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_TIMER:
        if (g_engine) g_engine->Update();
        break;

    case WM_PAINT:
    {
        if (g_engine) g_engine->Render();
        ValidateRect(hWnd, NULL);
    }
    break;

    case WM_DESTROY:
        if (g_engine) {
            delete g_engine;
            g_engine = nullptr;
        }
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
