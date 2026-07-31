#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <cmath>
#include "math.h"

struct ButtonState {
    const wchar_t* label;
    int x, y, width, height;
    float scale = 1.0f;
    float targetScale = 1.0f;
    COLORREF baseColor;
    COLORREF currentColor;
};

double g_firstOperand = 0.0;
wchar_t g_pendingOp = 0;
bool g_clearDisplay = false;
std::wstring g_displayText = L"0";

ButtonState g_buttons[] = {
    { L"C",  15,  80, 55, 55, 1.0f, 1.0f, RGB(80, 80, 85),   RGB(80, 80, 85) },
    { L"(",  75,  80, 55, 55, 1.0f, 1.0f, RGB(80, 80, 85),   RGB(80, 80, 85) },
    { L")",  135, 80, 55, 55, 1.0f, 1.0f, RGB(80, 80, 85),   RGB(80, 80, 85) },
    { L"/",  195, 80, 55, 55, 1.0f, 1.0f, RGB(255, 159, 10), RGB(255, 159, 10) },

    { L"7",  15,  140, 55, 55, 1.0f, 1.0f, RGB(45, 45, 50),  RGB(45, 45, 50) },
    { L"8",  75,  140, 55, 55, 1.0f, 1.0f, RGB(45, 45, 50),  RGB(45, 45, 50) },
    { L"9",  135, 140, 55, 55, 1.0f, 1.0f, RGB(45, 45, 50),  RGB(45, 45, 50) },
    { L"x",  195, 140, 55, 55, 1.0f, 1.0f, RGB(255, 159, 10), RGB(255, 159, 10) },

    { L"4",  15,  200, 55, 55, 1.0f, 1.0f, RGB(45, 45, 50),  RGB(45, 45, 50) },
    { L"5",  75,  200, 55, 55, 1.0f, 1.0f, RGB(45, 45, 50),  RGB(45, 45, 50) },
    { L"6",  135, 200, 55, 55, 1.0f, 1.0f, RGB(45, 45, 50),  RGB(45, 45, 50) },
    { L"-",  195, 200, 55, 55, 1.0f, 1.0f, RGB(255, 159, 10), RGB(255, 159, 10) },

    { L"1",  15,  260, 55, 55, 1.0f, 1.0f, RGB(45, 45, 50),  RGB(45, 45, 50) },
    { L"2",  75,  260, 55, 55, 1.0f, 1.0f, RGB(45, 45, 50),  RGB(45, 45, 50) },
    { L"3",  135, 260, 55, 55, 1.0f, 1.0f, RGB(45, 45, 50),  RGB(45, 45, 50) },
    { L"+",  195, 260, 55, 55, 1.0f, 1.0f, RGB(255, 159, 10), RGB(255, 159, 10) },

    { L"0",  15,  320, 115, 55, 1.0f, 1.0f, RGB(45, 45, 50), RGB(45, 45, 50) },
    { L".",  135, 320, 55, 55, 1.0f, 1.0f, RGB(45, 45, 50),  RGB(45, 45, 50) },
    { L"=",  195, 320, 55, 55, 1.0f, 1.0f, RGB(255, 149, 0), RGB(255, 149, 0) }
};

void HandleInput(const wchar_t* label) {
    if (label[0] >= L'0' && label[0] <= L'9') {
        if (g_displayText == L"0" || g_clearDisplay) {
            g_displayText = label;
            g_clearDisplay = false;
        }
        else {
            g_displayText += label;
        }
    }
    else if (wcscmp(label, L".") == 0) {
        if (g_displayText.find(L'.') == std::wstring::npos) {
            g_displayText += L".";
        }
    }
    else if (wcscmp(label, L"C") == 0) {
        g_displayText = L"0";
        g_firstOperand = 0.0;
        g_pendingOp = 0;
    }
    else if (wcscmp(label, L"+") == 0 || wcscmp(label, L"-") == 0 || wcscmp(label, L"x") == 0 || wcscmp(label, L"/") == 0) {
        g_firstOperand = _wtof(g_displayText.c_str());
        g_pendingOp = label[0];
        g_clearDisplay = true;
    }
    else if (wcscmp(label, L"=") == 0 && g_pendingOp != 0) {
        double secondOperand = _wtof(g_displayText.c_str());
        double result = 0.0;
        bool err = false;

        if (g_pendingOp == L'+') result = CalculatorMath::Add(g_firstOperand, secondOperand);
        else if (g_pendingOp == L'-') result = CalculatorMath::Subtract(g_firstOperand, secondOperand);
        else if (g_pendingOp == L'x') result = CalculatorMath::Multiply(g_firstOperand, secondOperand);
        else if (g_pendingOp == L'/') result = CalculatorMath::Divide(g_firstOperand, secondOperand, err);

        if (err) {
            g_displayText = L"Error";
        }
        else {
            wchar_t buf[64];
            swprintf_s(buf, 64, L"%.8g", result);
            g_displayText = buf;
        }
        g_pendingOp = 0;
        g_clearDisplay = true;
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        SetTimer(hWnd, 1, 16, NULL);
        break;

    case WM_TIMER: {
        bool needsRedraw = false;
        for (auto& btn : g_buttons) {
            if (std::abs(btn.scale - btn.targetScale) > 0.001f) {
                btn.scale += (btn.targetScale - btn.scale) * 0.25f;
                needsRedraw = true;
            }
        }
        if (needsRedraw) InvalidateRect(hWnd, NULL, FALSE);
        break;
    }

    case WM_LBUTTONDOWN: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        for (auto& btn : g_buttons) {
            if (x >= btn.x && x <= btn.x + btn.width && y >= btn.y && y <= btn.y + btn.height) {
                btn.scale = 0.88f;
                HandleInput(btn.label);
                InvalidateRect(hWnd, NULL, FALSE);
                break;
            }
        }
        break;
    }

    case WM_LBUTTONUP: {
        for (auto& btn : g_buttons) {
            btn.targetScale = 1.0f;
        }
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        HDC memDC = CreateCompatibleDC(hdc);
        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
        SelectObject(memDC, memBitmap);

        HBRUSH bgBrush = CreateSolidBrush(RGB(18, 18, 20));
        FillRect(memDC, &clientRect, bgBrush);
        DeleteObject(bgBrush);

        SetBkMode(memDC, TRANSPARENT);
        SetTextColor(memDC, RGB(255, 255, 255));
        HFONT displayFont = CreateFontW(36, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        SelectObject(memDC, displayFont);

        RECT dispRect = { 15, 15, 250, 65 };
        DrawTextW(memDC, g_displayText.c_str(), -1, &dispRect, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
        DeleteObject(displayFont);

        HFONT btnFont = CreateFontW(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        SelectObject(memDC, btnFont);

        for (const auto& btn : g_buttons) {
            int w = (int)(btn.width * btn.scale);
            int h = (int)(btn.height * btn.scale);
            int ox = btn.x + (btn.width - w) / 2;
            int oy = btn.y + (btn.height - h) / 2;

            HBRUSH btnBrush = CreateSolidBrush(btn.currentColor);
            HPEN nullPen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
            SelectObject(memDC, btnBrush);
            SelectObject(memDC, nullPen);

            RoundRect(memDC, ox, oy, ox + w, oy + h, 14, 14);

            RECT textRect = { ox, oy, ox + w, oy + h };
            DrawTextW(memDC, btn.label, -1, &textRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

            DeleteObject(btnBrush);
            DeleteObject(nullPen);
        }
        DeleteObject(btnFont);

        BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, memDC, 0, 0, SRCCOPY);
        DeleteObject(memBitmap);
        DeleteDC(memDC);
        EndPaint(hWnd, &ps);
        break;
    }

    case WM_DESTROY:
        KillTimer(hWnd, 1);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = L"AnimatedCalc";

    RegisterClassW(&wc);
    HWND hWnd = CreateWindowW(L"AnimatedCalc", L"Calculator", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 280, 420, NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}