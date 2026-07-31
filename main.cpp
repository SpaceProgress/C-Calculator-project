#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <string>
#include <cmath>
#include "math.hpp"

struct Button {
    const wchar_t* label;
    int x, y, width, height;
    COLORREF idleColor;
    COLORREF hoverColor;
    COLORREF pressColor;

    float currentScale = 1.0f;
    float targetScale = 1.0f;
    float scaleVelocity = 0.0f;

    float currentR = 0.0f, currentG = 0.0f, currentB = 0.0f;
    float targetR = 0.0f, targetG = 0.0f, targetB = 0.0f;

    bool isHovered = false;
    bool isPressed = false;
};

double g_firstOperand = 0.0;
wchar_t g_pendingOp = 0;
bool g_clearDisplay = false;
std::wstring g_displayText = L"0";

float g_displayAnimY = 0.0f;
float g_displayTargetY = 0.0f;
float g_displayVelocityY = 0.0f;

Button g_buttons[] = {
    { L"C",   18, 115, 58, 58, RGB(90, 90, 98),   RGB(115, 115, 125), RGB(70, 70, 78) },
    { L"+/-", 82, 115, 58, 58, RGB(90, 90, 98),   RGB(115, 115, 125), RGB(70, 70, 78) },
    { L"%",  146, 115, 58, 58, RGB(90, 90, 98),   RGB(115, 115, 125), RGB(70, 70, 78) },
    { L"/",  210, 115, 58, 58, RGB(255, 159, 10), RGB(255, 180, 60),  RGB(210, 125, 0) },

    { L"7",   18, 181, 58, 58, RGB(48, 48, 54),   RGB(72, 72, 80),    RGB(35, 35, 40) },
    { L"8",   82, 181, 58, 58, RGB(48, 48, 54),   RGB(72, 72, 80),    RGB(35, 35, 40) },
    { L"9",  146, 181, 58, 58, RGB(48, 48, 54),   RGB(72, 72, 80),    RGB(35, 35, 40) },
    { L"x",  210, 181, 58, 58, RGB(255, 159, 10), RGB(255, 180, 60),  RGB(210, 125, 0) },

    { L"4",   18, 247, 58, 58, RGB(48, 48, 54),   RGB(72, 72, 80),    RGB(35, 35, 40) },
    { L"5",   82, 247, 58, 58, RGB(48, 48, 54),   RGB(72, 72, 80),    RGB(35, 35, 40) },
    { L"6",  146, 247, 58, 58, RGB(48, 48, 54),   RGB(72, 72, 80),    RGB(35, 35, 40) },
    { L"-",  210, 247, 58, 58, RGB(255, 159, 10), RGB(255, 180, 60),  RGB(210, 125, 0) },

    { L"1",   18, 313, 58, 58, RGB(48, 48, 54),   RGB(72, 72, 80),    RGB(35, 35, 40) },
    { L"2",   82, 313, 58, 58, RGB(48, 48, 54),   RGB(72, 72, 80),    RGB(35, 35, 40) },
    { L"3",  146, 313, 58, 58, RGB(48, 48, 54),   RGB(72, 72, 80),    RGB(35, 35, 40) },
    { L"+",  210, 313, 58, 58, RGB(255, 159, 10), RGB(255, 180, 60),  RGB(210, 125, 0) },

    { L"0",   18, 379, 122, 58, RGB(48, 48, 54),  RGB(72, 72, 80),    RGB(35, 35, 40) },
    { L".",  146, 379, 58, 58, RGB(48, 48, 54),   RGB(72, 72, 80),    RGB(35, 35, 40) },
    { L"=",  210, 379, 58, 58, RGB(255, 159, 10), RGB(255, 180, 60),  RGB(210, 125, 0) }
};

void TriggerDisplayAnimation() {
    g_displayAnimY = 16.0f;
    g_displayTargetY = 0.0f;
    g_displayVelocityY = 0.0f;
}

void ExecuteCalculatorAction(const wchar_t* label) {
    TriggerDisplayAnimation();

    if (label[0] >= L'0' && label[0] <= L'9') {
        if (g_displayText == L"0" || g_clearDisplay) {
            g_displayText = label;
            g_clearDisplay = false;
        }
        else if (g_displayText.length() < 11) {
            g_displayText += label;
        }
    }
    else if (wcscmp(label, L".") == 0) {
        if (g_clearDisplay) {
            g_displayText = L"0.";
            g_clearDisplay = false;
        }
        else if (g_displayText.find(L'.') == std::wstring::npos) {
            g_displayText += L".";
        }
    }
    else if (wcscmp(label, L"C") == 0) {
        g_displayText = L"0";
        g_firstOperand = 0.0;
        g_pendingOp = 0;
        g_clearDisplay = false;
    }
    else if (wcscmp(label, L"+/-") == 0) {
        if (g_displayText != L"0" && g_displayText != L"Error") {
            if (g_displayText[0] == L'-') g_displayText.erase(0, 1);
            else g_displayText.insert(0, L"-");
        }
    }
    else if (wcscmp(label, L"%") == 0) {
        double val = _wtof(g_displayText.c_str());
        val = CalculatorMath::Multiply(val, 0.01);
        wchar_t buf[64];
        swprintf_s(buf, 64, L"%.8g", val);
        g_displayText = buf;
    }
    else if (wcscmp(label, L"+") == 0 || wcscmp(label, L"-") == 0 || wcscmp(label, L"x") == 0 || wcscmp(label, L"/") == 0) {
        g_firstOperand = _wtof(g_displayText.c_str());
        g_pendingOp = label[0];
        g_clearDisplay = true;
    }
    else if (wcscmp(label, L"=") == 0 && g_pendingOp != 0) {
        double secondOperand = _wtof(g_displayText.c_str());
        try {
            double res = 0.0;
            if (g_pendingOp == L'+') res = CalculatorMath::Add(g_firstOperand, secondOperand);
            else if (g_pendingOp == L'-') res = CalculatorMath::Subtract(g_firstOperand, secondOperand);
            else if (g_pendingOp == L'x') res = CalculatorMath::Multiply(g_firstOperand, secondOperand);
            else if (g_pendingOp == L'/') res = CalculatorMath::Divide(g_firstOperand, secondOperand);

            wchar_t buf[64];
            swprintf_s(buf, 64, L"%.8g", res);
            g_displayText = buf;
        }
        catch (...) {
            g_displayText = L"Stupid Fuck ";
        }
        g_pendingOp = 0;
        g_clearDisplay = true;
    }
}

void SetColorTarget(Button& btn, COLORREF target) {
    btn.targetR = static_cast<float>(GetRValue(target));
    btn.targetG = static_cast<float>(GetGValue(target));
    btn.targetB = static_cast<float>(GetBValue(target));
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        for (auto& btn : g_buttons) {
            SetColorTarget(btn, btn.idleColor);
            btn.currentR = btn.targetR;
            btn.currentG = btn.targetG;
            btn.currentB = btn.targetB;
        }
        SetTimer(hWnd, 1, 16, NULL);
        break;
    }

    case WM_TIMER: {
        bool needsRedraw = false;

        for (auto& btn : g_buttons) {
            float force = (btn.targetScale - btn.currentScale) * 0.25f;
            btn.scaleVelocity = (btn.scaleVelocity + force) * 0.65f;
            btn.currentScale += btn.scaleVelocity;

            if (std::abs(btn.scaleVelocity) > 0.0001f || std::abs(btn.targetScale - btn.currentScale) > 0.0001f) {
                needsRedraw = true;
            }

            float rDiff = btn.targetR - btn.currentR;
            float gDiff = btn.targetG - btn.currentG;
            float bDiff = btn.targetB - btn.currentB;

            if (std::abs(rDiff) > 0.1f || std::abs(gDiff) > 0.1f || std::abs(bDiff) > 0.1f) {
                btn.currentR += rDiff * 0.2f;
                btn.currentG += gDiff * 0.2f;
                btn.currentB += bDiff * 0.2f;
                needsRedraw = true;
            }
        }

        float dispForce = (g_displayTargetY - g_displayAnimY) * 0.28f;
        g_displayVelocityY = (g_displayVelocityY + dispForce) * 0.60f;
        g_displayAnimY += g_displayVelocityY;

        if (std::abs(g_displayVelocityY) > 0.001f || std::abs(g_displayTargetY - g_displayAnimY) > 0.001f) {
            needsRedraw = true;
        }

        if (needsRedraw) {
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;
    }

    case WM_MOUSEMOVE: {
        int mx = GET_X_LPARAM(lParam);
        int my = GET_Y_LPARAM(lParam);

        TRACKMOUSEEVENT tme = {};
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = hWnd;
        TrackMouseEvent(&tme);

        for (auto& btn : g_buttons) {
            bool hover = (mx >= btn.x && mx <= btn.x + btn.width && my >= btn.y && my <= btn.y + btn.height);
            if (hover != btn.isHovered) {
                btn.isHovered = hover;
                if (!btn.isPressed) {
                    btn.targetScale = hover ? 1.06f : 1.0f;
                    SetColorTarget(btn, hover ? btn.hoverColor : btn.idleColor);
                }
            }
        }
        break;
    }

    case WM_MOUSELEAVE: {
        for (auto& btn : g_buttons) {
            btn.isHovered = false;
            btn.isPressed = false;
            btn.targetScale = 1.0f;
            SetColorTarget(btn, btn.idleColor);
        }
        break;
    }

    case WM_LBUTTONDOWN: {
        int mx = GET_X_LPARAM(lParam);
        int my = GET_Y_LPARAM(lParam);

        for (auto& btn : g_buttons) {
            if (mx >= btn.x && mx <= btn.x + btn.width && my >= btn.y && my <= btn.y + btn.height) {
                btn.isPressed = true;
                btn.targetScale = 0.90f;
                SetColorTarget(btn, btn.pressColor);
                ExecuteCalculatorAction(btn.label);
                SetCapture(hWnd);
                break;
            }
        }
        break;
    }

    case WM_LBUTTONUP: {
        ReleaseCapture();
        for (auto& btn : g_buttons) {
            if (btn.isPressed) {
                btn.isPressed = false;
                btn.targetScale = btn.isHovered ? 1.06f : 1.0f;
                SetColorTarget(btn, btn.isHovered ? btn.hoverColor : btn.idleColor);
            }
        }
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        int width = clientRect.right;
        int height = clientRect.bottom;

        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
        SelectObject(memDC, memBitmap);

        HBRUSH bgBrush = CreateSolidBrush(RGB(20, 20, 24));
        FillRect(memDC, &clientRect, bgBrush);
        DeleteObject(bgBrush);

        SetBkMode(memDC, TRANSPARENT);
        SetTextColor(memDC, RGB(250, 250, 250));

        HFONT dispFont = CreateFontW(46, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_SWISS, L"Segoe UI Variable Display");
        SelectObject(memDC, dispFont);

        RECT dispRect = { 18, 20 + static_cast<int>(g_displayAnimY), 268, 98 + static_cast<int>(g_displayAnimY) };
        DrawTextW(memDC, g_displayText.c_str(), -1, &dispRect, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
        DeleteObject(dispFont);

        HFONT btnFont = CreateFontW(22, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_SWISS, L"Segoe UI Text");
        SelectObject(memDC, btnFont);

        for (const auto& btn : g_buttons) {
            int w = static_cast<int>(btn.width * btn.currentScale);
            int h = static_cast<int>(btn.height * btn.currentScale);
            int ox = btn.x + (btn.width - w) / 2;
            int oy = btn.y + (btn.height - h) / 2;

            COLORREF fillCol = RGB(static_cast<BYTE>(btn.currentR), static_cast<BYTE>(btn.currentG), static_cast<BYTE>(btn.currentB));
            HBRUSH btnBrush = CreateSolidBrush(fillCol);
            HPEN nullPen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));

            SelectObject(memDC, btnBrush);
            SelectObject(memDC, nullPen);

            RoundRect(memDC, ox, oy, ox + w, oy + h, 28, 28);

            RECT textRect = { ox, oy, ox + w, oy + h };
            DrawTextW(memDC, btn.label, -1, &textRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

            DeleteObject(btnBrush);
            DeleteObject(nullPen);
        }

        DeleteObject(btnFont);

        BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);
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
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wc.lpszClassName = L"SmoothCalcWindow";

    RegisterClassW(&wc);

    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT r = { 0, 0, 286, 455 };
    AdjustWindowRect(&r, style, FALSE);

    HWND hWnd = CreateWindowW(L"SmoothCalcWindow", L"Calculator", style,
        CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
}
