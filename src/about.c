#include "about.h"
#include <windowsx.h>
#include <shellapi.h>

#define ABOUT_W  420
#define ABOUT_H  400
#define BTN_W    80
#define BTN_H    28

static const wchar_t *kTitle     = L"Snipshot v1.0.0";
static const wchar_t *kScreenshot = L"Screenshot:";
static const wchar_t *kDrawBoard  = L"Drawing Board:";
static const wchar_t *kAuthor    = L"Author: Eetherrr";
static const wchar_t *kGitHub    = L"github.com/Etherre/snipshot";

static const wchar_t *kShotLines[] = {
    L"Ctrl+Shift+S  —  Screenshot + Pin",
    L"Ctrl+Shift+A  —  Pin + Copy to Clipboard",
    L"Ctrl+Shift+D  —  Screenshot + Copy only",
};
static const wchar_t *kDrawLines[] = {
    L"Ctrl+Shift+B  —  Enter Drawing Board",
    L"Left button   —  Pen",
    L"Right button  —  Eraser",
    L"Mouse wheel   —  Adjust pen size",
    L"Hold Alt      —  Mouse passthrough",
    L"Esc           —  Exit Drawing Board",
};

// 绘制区域内 GitHub 链接的命中测试矩形
static RECT g_githubRect;

static void OpenURL(const wchar_t *url)
{
    ShellExecuteW(NULL, L"open", url, NULL, NULL, SW_SHOW);
}

LRESULT CALLBACK AboutProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CREATE:
            return 0;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT client;
            GetClientRect(hwnd, &client);

            // 背景
            HBRUSH bg = CreateSolidBrush(RGB(40, 40, 43));
            FillRect(hdc, &client, bg);
            DeleteObject(bg);

            SetBkMode(hdc, TRANSPARENT);
            HFONT titleFont = CreateFontW(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                          CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                          DEFAULT_PITCH, L"Segoe UI");
            HFONT sectionFont = CreateFontW(14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                            DEFAULT_PITCH, L"Segoe UI");
            HFONT bodyFont = CreateFontW(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                         CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                         DEFAULT_PITCH, L"Segoe UI");

            int y = 16;

            // 标题
            SelectObject(hdc, titleFont);
            SetTextColor(hdc, RGB(255, 255, 255));
            RECT tr = {20, y, client.right - 20, y + 30};
            DrawTextW(hdc, kTitle, -1, &tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            y += 40;

            // ── Screenshot 区块 ──
            SelectObject(hdc, sectionFont);
            SetTextColor(hdc, RGB(0, 180, 220));
            RECT sr = {24, y, client.right - 24, y + 22};
            DrawTextW(hdc, kScreenshot, -1, &sr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            y += 24;
            SelectObject(hdc, bodyFont);
            SetTextColor(hdc, RGB(210, 210, 214));
            for (int i = 0; i < 3; i++) {
                RECT lr = {36, y, client.right - 36, y + 20};
                DrawTextW(hdc, kShotLines[i], -1, &lr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                y += 20;
            }
            y += 8;

            // ── Drawing Board 区块 ──
            SelectObject(hdc, sectionFont);
            SetTextColor(hdc, RGB(0, 180, 220));
            RECT dr = {24, y, client.right - 24, y + 22};
            DrawTextW(hdc, kDrawBoard, -1, &dr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            y += 24;
            SelectObject(hdc, bodyFont);
            SetTextColor(hdc, RGB(210, 210, 214));
            for (int i = 0; i < 6; i++) {
                RECT lr = {36, y, client.right - 36, y + 20};
                DrawTextW(hdc, kDrawLines[i], -1, &lr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                y += 20;
            }
            y += 8;

            // ── 分隔线 ──
            HPEN sep = CreatePen(PS_SOLID, 1, RGB(80, 80, 84));
            SelectObject(hdc, sep);
            MoveToEx(hdc, 20, y, NULL);
            LineTo(hdc, client.right - 20, y);
            DeleteObject(sep);
            y += 10;

            // ── 作者与 GitHub ──
            SelectObject(hdc, bodyFont);
            SetTextColor(hdc, RGB(180, 180, 184));
            RECT ar = {20, y, client.right - 20, y + 20};
            DrawTextW(hdc, kAuthor, -1, &ar, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            y += 20;

            // GitHub 链接（蓝色下划线）
            SetTextColor(hdc, RGB(80, 160, 255));
            RECT gr = {20, y, client.right - 20, y + 20};
            DrawTextW(hdc, kGitHub, -1, &gr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            // 记录点击区域
            DrawTextW(hdc, kGitHub, -1, &gr, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
            g_githubRect = gr;
            // 下划线
            int tw = gr.right - gr.left;
            HPEN ul = CreatePen(PS_SOLID, 1, RGB(80, 160, 255));
            SelectObject(hdc, ul);
            MoveToEx(hdc, gr.left, gr.bottom - 1, NULL);
            LineTo(hdc, gr.left + tw, gr.bottom - 1);
            DeleteObject(ul);

            y = client.bottom - BTN_H - 16;

            // ── OK 按钮 ──
            RECT br = {(client.right - BTN_W) / 2, y,
                       (client.right + BTN_W) / 2, y + BTN_H};
            HBRUSH btnFace = CreateSolidBrush(RGB(0, 120, 212));
            FillRect(hdc, &br, btnFace);
            DeleteObject(btnFace);
            SetTextColor(hdc, RGB(255, 255, 255));
            DrawTextW(hdc, L"OK", -1, &br, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            DeleteObject(titleFont);
            DeleteObject(sectionFont);
            DeleteObject(bodyFont);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN:
        {
            int mx = GET_X_LPARAM(lParam);
            int my = GET_Y_LPARAM(lParam);
            POINT pt = {mx, my};

            // 点击 GitHub 链接 → 打开浏览器
            if (PtInRect(&g_githubRect, pt)) {
                OpenURL(L"https://github.com/Etherre/snipshot");
                return 0;
            }

            // 点击 OK 按钮区域
            RECT br = {(ABOUT_W - BTN_W) / 2, ABOUT_H - BTN_H - 16,
                       (ABOUT_W + BTN_W) / 2, ABOUT_H - 16};
            if (PtInRect(&br, pt)) {
                DestroyWindow(hwnd);
                return 0;
            }
            return 0;
        }

        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE)
                DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

BOOL RegisterAboutClass(HINSTANCE hInst)
{
    WNDCLASSW wc = {0};
    wc.hInstance     = hInst;
    wc.lpfnWndProc   = AboutProc;
    wc.lpszClassName = L"SnipshotAbout";
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    return RegisterClassW(&wc);
}

void ShowAboutDialog(HINSTANCE hInst)
{
    int cx = GetSystemMetrics(SM_CXSCREEN);
    int cy = GetSystemMetrics(SM_CYSCREEN);
    int x = (cx - ABOUT_W) / 2;
    int y = (cy - ABOUT_H) / 2;

    HWND hDlg = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"SnipshotAbout", L"About Snipshot",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        x, y, ABOUT_W, ABOUT_H,
        NULL, NULL, hInst, NULL);

    if (!hDlg) return;

    MSG msg;
    while (IsWindow(hDlg) && GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}