#include "about.h"
#include "hotkey.h"
#include <windowsx.h>
#include <shellapi.h>

#define ABOUT_W  440
#define ABOUT_H  460
#define BTN_W    80
#define BTN_H    28

static const wchar_t *kTitle    = L"Snipshot v1.2.0";
static const wchar_t *kSubtitle = L"Lightweight screenshot & screen annotation tool for Windows";

static const wchar_t *kShotLabel = L"Screenshot";
static const wchar_t *kDrawLabel = L"Drawing Board";
static const wchar_t *kSettingsNote = L"Shortcuts can be customized in Settings.";

static const wchar_t *kShotKeys[3]  = { L"Pin",       L"Pin + Copy",        L"Copy" };
static const wchar_t *kDrawKeys[5]  = { L"Enter",      L"Draw",      L"Erase",
                                        L"Adjust size", L"Passthrough" };
static const wchar_t *kDrawVals[5]  = { L"Ctrl+Shift+B", L"Left button",
                                        L"Right button (hold)",
                                        L"Mouse wheel", L"Hold Alt" };

static const wchar_t *kAuthor  = L"Author: Eetherrr";
static const wchar_t *kGitHub  = L"github.com/Etherre/snipshot";

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

            // 字体
            HFONT titleFont = CreateFontW(20, 0, 0, 0, FW_BOLD, FALSE,FALSE,FALSE,
                                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                          CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                          DEFAULT_PITCH, L"Segoe UI");
            HFONT sectionFont = CreateFontW(14, 0, 0, 0, FW_SEMIBOLD, FALSE,FALSE,FALSE,
                                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                            DEFAULT_PITCH, L"Segoe UI");
            HFONT bodyFont = CreateFontW(13, 0, 0, 0, FW_NORMAL, FALSE,FALSE,FALSE,
                                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                         CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                         DEFAULT_PITCH, L"Segoe UI");
            HFONT noteFont = CreateFontW(12, 0, 0, 0, FW_NORMAL, FALSE,FALSE,FALSE,
                                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                         CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                         DEFAULT_PITCH, L"Segoe UI");

            int y = 16;

            // 标题
            SelectObject(hdc, titleFont);
            SetTextColor(hdc, RGB(255, 255, 255));
            RECT tr = {20, y, client.right - 20, y + 30};
            DrawTextW(hdc, kTitle, -1, &tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            y += 32;

            // 副标题
            SelectObject(hdc, noteFont);
            SetTextColor(hdc, RGB(150, 150, 156));
            RECT sr = {20, y, client.right - 20, y + 18};
            DrawTextW(hdc, kSubtitle, -1, &sr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            y += 28;

            // ── Screenshot 区块 ──
            {
                // 区块标题 + 背景条
                RECT sh = {16, y, client.right - 16, y + 26};
                HBRUSH shBr = CreateSolidBrush(RGB(52, 52, 56));
                FillRect(hdc, &sh, shBr);
                DeleteObject(shBr);

                SelectObject(hdc, sectionFont);
                SetTextColor(hdc, RGB(80, 200, 240));
                RECT st = {28, y, client.right - 28, y + 26};
                DrawTextW(hdc, kShotLabel, -1, &st, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                y += 34;

                // 快捷鍵行
                SelectObject(hdc, bodyFont);
                for (int i = 0; i < 3; i++) {
                    SetTextColor(hdc, RGB(200, 200, 206));
                    RECT lr = {32, y, 200, y + 20};
                    DrawTextW(hdc, kShotKeys[i], -1, &lr,
                              DT_LEFT | DT_VCENTER | DT_SINGLELINE);

                    wchar_t hk[48];
                    FormatHotkey(
                        GetHotkeyActions()[i].defMod,
                        GetHotkeyActions()[i].defVk, hk, 48);
                    SetTextColor(hdc, RGB(180, 180, 186));
                    RECT rr = {200, y, client.right - 32, y + 20};
                    DrawTextW(hdc, hk, -1, &rr, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
                    y += 20;
                }
            }
            y += 10;

            // ── Drawing Board 区块 ──
            {
                RECT dh = {16, y, client.right - 16, y + 26};
                HBRUSH dhBr = CreateSolidBrush(RGB(52, 52, 56));
                FillRect(hdc, &dh, dhBr);
                DeleteObject(dhBr);

                SelectObject(hdc, sectionFont);
                SetTextColor(hdc, RGB(80, 200, 240));
                RECT dt = {28, y, client.right - 28, y + 26};
                DrawTextW(hdc, kDrawLabel, -1, &dt, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                y += 34;

                SelectObject(hdc, bodyFont);
                for (int i = 0; i < 5; i++) {
                    SetTextColor(hdc, RGB(200, 200, 206));
                    RECT lr = {32, y, 200, y + 20};
                    DrawTextW(hdc, kDrawKeys[i], -1, &lr,
                              DT_LEFT | DT_VCENTER | DT_SINGLELINE);

                    SetTextColor(hdc, RGB(180, 180, 186));
                    RECT rr = {200, y, client.right - 32, y + 20};
                    DrawTextW(hdc, kDrawVals[i], -1, &rr,
                              DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
                    y += 20;
                }
            }
            y += 10;

            // 设置提示
            SelectObject(hdc, noteFont);
            SetTextColor(hdc, RGB(130, 130, 136));
            RECT nr = {20, y, client.right - 20, y + 18};
            DrawTextW(hdc, kSettingsNote, -1, &nr,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            y += 20;

            // 分隔线
            y += 6;
            HPEN sep = CreatePen(PS_SOLID, 1, RGB(70, 70, 74));
            SelectObject(hdc, sep);
            MoveToEx(hdc, 20, y, NULL);
            LineTo(hdc, client.right - 20, y);
            DeleteObject(sep);
            y += 12;

            // 作者
            SelectObject(hdc, bodyFont);
            SetTextColor(hdc, RGB(180, 180, 184));
            RECT ar = {20, y, client.right - 20, y + 20};
            DrawTextW(hdc, kAuthor, -1, &ar, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            y += 20;

            // GitHub 链接
            SetTextColor(hdc, RGB(80, 160, 255));
            RECT gr = {20, y, client.right - 20, y + 20};
            DrawTextW(hdc, kGitHub, -1, &gr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            DrawTextW(hdc, kGitHub, -1, &gr,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
            g_githubRect = gr;
            // 下划线
            int tw = gr.right - gr.left;
            HPEN ul = CreatePen(PS_SOLID, 1, RGB(80, 160, 255));
            SelectObject(hdc, ul);
            MoveToEx(hdc, gr.left, gr.bottom - 1, NULL);
            LineTo(hdc, gr.left + tw, gr.bottom - 1);
            DeleteObject(ul);

            // OK 按钮
            y = client.bottom - BTN_H - 16;
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
            DeleteObject(noteFont);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN:
        {
            int mx = GET_X_LPARAM(lParam);
            int my = GET_Y_LPARAM(lParam);
            POINT pt = {mx, my};

            if (PtInRect(&g_githubRect, pt)) {
                OpenURL(L"https://github.com/Etherre/snipshot");
                return 0;
            }

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