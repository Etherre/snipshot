#include "settings.h"
#include "hotkey.h"
#include <windowsx.h>

#define DIALOG_W  440
#define DIALOG_H  290
#define ROW_Y0    48
#define ROW_H     38
#define LABEL_X  24
#define HOTKEY_X 240
#define HOTKEY_W 170
#define BTN_OK_W  80
#define BTN_RST_W 140
#define BTN_H     28

static BOOL     g_changed = FALSE;
static UINT     g_mods[HK_COUNT];
static UINT     g_vks[HK_COUNT];
static BOOL     g_capturing = FALSE;
static int      g_capIndex = -1;
static UINT     g_capMods = 0;

// 每行快捷键的命中测试矩形
static RECT g_hkRects[HK_COUNT];

// 防重入：对话框已在显示时聚焦并返回（托盘菜单在模态循环内仍可触发）
static HWND s_settingsHwnd = NULL;

static BOOL IsModifier(UINT vk)
{
    return vk == VK_CONTROL || vk == VK_SHIFT ||
           vk == VK_MENU || vk == VK_LMENU || vk == VK_RMENU;
}

static BOOL IsDuplicate(int idx, UINT mods, UINT vk)
{
    for (int i = 0; i < HK_COUNT; i++) {
        if (i != idx && g_mods[i] == mods && g_vks[i] == vk)
            return TRUE;
    }
    return FALSE;
}

LRESULT CALLBACK SettingsProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CREATE:
        {
            const HotkeyAction *actions = GetHotkeyActions();
            for (int i = 0; i < HK_COUNT; i++) {
                g_mods[i] = actions[i].defMod;
                g_vks[i]  = actions[i].defVk;
            }
            LoadHotkeys(g_mods, g_vks);
            g_changed = FALSE;
            g_capturing = FALSE;
            g_capIndex = -1;
            return 0;
        }

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

            // 标题
            HFONT titleFont = CreateFontW(18, 0, 0, 0, FW_BOLD, FALSE,FALSE,FALSE,
                                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                          CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                          DEFAULT_PITCH, L"Segoe UI");
            SelectObject(hdc, titleFont);
            SetTextColor(hdc, RGB(255, 255, 255));
            RECT tr = {20, 12, client.right - 20, 40};
            DrawTextW(hdc, L"Keyboard Shortcuts", -1, &tr,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            DeleteObject(titleFont);

            // 操作行
            HFONT labelFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE,FALSE,FALSE,
                                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                          CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                          DEFAULT_PITCH, L"Segoe UI");
            HFONT hkFont = CreateFontW(13, 0, 0, 0, FW_NORMAL, FALSE,FALSE,FALSE,
                                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                       CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                       DEFAULT_PITCH, L"Consolas");

            const HotkeyAction *actions = GetHotkeyActions();

            for (int i = 0; i < HK_COUNT; i++) {
                int y = ROW_Y0 + i * ROW_H;

                // 标签
                SelectObject(hdc, labelFont);
                SetTextColor(hdc, RGB(210, 210, 214));
                RECT lr = {LABEL_X, y, HOTKEY_X - 12, y + 22};
                DrawTextW(hdc, actions[i].label, -1, &lr,
                          DT_LEFT | DT_VCENTER | DT_SINGLELINE);

                // 快捷键区域背景
                RECT hr = {HOTKEY_X, y + 2, HOTKEY_X + HOTKEY_W, y + 28};
                g_hkRects[i] = hr;

                BOOL active = (g_capturing && g_capIndex == i);
                COLORREF hkBg = active ? RGB(0, 90, 160) : RGB(58, 58, 62);
                HBRUSH hkBr = CreateSolidBrush(hkBg);
                FillRect(hdc, &hr, hkBr);
                DeleteObject(hkBr);

                // 边框
                HPEN border = CreatePen(PS_SOLID, 1,
                                        active ? RGB(80, 180, 255) : RGB(100, 100, 104));
                HBRUSH nullBr = GetStockObject(NULL_BRUSH);
                SelectObject(hdc, border);
                SelectObject(hdc, nullBr);
                Rectangle(hdc, hr.left, hr.top, hr.right, hr.bottom);
                DeleteObject(border);

                // 快捷键文字
                SelectObject(hdc, hkFont);
                wchar_t buf[48];
                if (g_capturing && g_capIndex == i) {
                    wcscpy(buf, L"...");
                    SetTextColor(hdc, RGB(200, 200, 200));
                } else {
                    FormatHotkey(g_mods[i], g_vks[i], buf, 48);
                    SetTextColor(hdc, RGB(240, 240, 240));
                }
                DrawTextW(hdc, buf, -1, &hr,
                          DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }

            DeleteObject(labelFont);
            DeleteObject(hkFont);

            // ── 底部按钮 ──
            int btnY = DIALOG_H - BTN_H - 20;

            // Reset 按钮
            RECT rstR = {DIALOG_W / 2 - BTN_RST_W - 60, btnY,
                         DIALOG_W / 2 - 60, btnY + BTN_H};
            HBRUSH rstFace = CreateSolidBrush(RGB(70, 70, 74));
            FillRect(hdc, &rstR, rstFace);
            DeleteObject(rstFace);
            HPEN rstPen = CreatePen(PS_SOLID, 1, RGB(120, 120, 124));
            SelectObject(hdc, rstPen);
            SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Rectangle(hdc, rstR.left, rstR.top, rstR.right, rstR.bottom);
            DeleteObject(rstPen);
            SetTextColor(hdc, RGB(220, 220, 224));
            DrawTextW(hdc, L"Reset Defaults", -1, &rstR,
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // OK 按钮
            RECT okR = {DIALOG_W / 2 + 60, btnY,
                        DIALOG_W / 2 + 60 + BTN_OK_W, btnY + BTN_H};
            HBRUSH okFace = CreateSolidBrush(RGB(0, 120, 212));
            FillRect(hdc, &okR, okFace);
            DeleteObject(okFace);
            SetTextColor(hdc, RGB(255, 255, 255));
            DrawTextW(hdc, L"OK", -1, &okR,
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN:
        {
            int mx = GET_X_LPARAM(lParam);
            int my = GET_Y_LPARAM(lParam);
            POINT pt = {mx, my};

            // 检查是否点击快捷键区域
            for (int i = 0; i < HK_COUNT; i++) {
                if (PtInRect(&g_hkRects[i], pt)) {
                    g_capturing = TRUE;
                    g_capIndex = i;
                    g_capMods = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
            }

            // Reset 按钮
            RECT rstR = {DIALOG_W / 2 - BTN_RST_W - 60,
                         DIALOG_H - BTN_H - 20,
                         DIALOG_W / 2 - 60,
                         DIALOG_H - 20};
            if (PtInRect(&rstR, pt)) {
                g_capturing = FALSE;
                g_capIndex = -1;
                ResetHotkeys();
                LoadHotkeys(g_mods, g_vks);
                g_changed = TRUE;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }

            // OK 按钮
            RECT okR = {DIALOG_W / 2 + 60,
                        DIALOG_H - BTN_H - 20,
                        DIALOG_W / 2 + 60 + BTN_OK_W,
                        DIALOG_H - 20};
            if (PtInRect(&okR, pt)) {
                DestroyWindow(hwnd);
                return 0;
            }

            // 点击空白处退出捕获模式
            if (g_capturing) {
                g_capturing = FALSE;
                g_capIndex = -1;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        {
            UINT vk = (UINT)wParam;

            // Esc 取消捕获或关闭
            if (vk == VK_ESCAPE) {
                if (g_capturing) {
                    g_capturing = FALSE;
                    g_capIndex = -1;
                    InvalidateRect(hwnd, NULL, FALSE);
                } else {
                    DestroyWindow(hwnd);
                }
                return 0;
            }

            if (!g_capturing)
                return 0;

            // 跟踪修饰键状态
            if (IsModifier(vk)) {
                if (vk == VK_CONTROL)    g_capMods |= MOD_CONTROL;
                if (vk == VK_SHIFT)      g_capMods |= MOD_SHIFT;
                if (vk == VK_MENU || vk == VK_LMENU || vk == VK_RMENU)
                    g_capMods |= MOD_ALT;
                return 0;
            }

            // 非修饰键 → 完成捕获
            g_capMods &= (MOD_CONTROL | MOD_SHIFT | MOD_ALT);

            if (IsDuplicate(g_capIndex, g_capMods, vk)) {
                // 重复快捷键：忽略
                g_capturing = FALSE;
                g_capIndex = -1;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }

            // 保存新快捷键
            g_mods[g_capIndex] = g_capMods;
            g_vks[g_capIndex] = vk;
            SaveHotkey(GetHotkeyActions()[g_capIndex].name,
                       g_capMods, vk);
            g_changed = TRUE;
            g_capturing = FALSE;
            g_capIndex = -1;
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            UINT vk = (UINT)wParam;
            if (vk == VK_CONTROL)    g_capMods &= ~MOD_CONTROL;
            if (vk == VK_SHIFT)      g_capMods &= ~MOD_SHIFT;
            if (vk == VK_MENU || vk == VK_LMENU || vk == VK_RMENU)
                g_capMods &= ~MOD_ALT;
            return 0;
        }

        case WM_DESTROY:
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

BOOL RegisterSettingsClass(HINSTANCE hInst)
{
    WNDCLASSW wc = {0};
    wc.hInstance     = hInst;
    wc.lpfnWndProc   = SettingsProc;
    wc.lpszClassName = L"SnipshotSettings";
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    return RegisterClassW(&wc);
}

BOOL ShowSettingsDialog(HINSTANCE hInst)
{
    // 防重入：已在显示时聚焦并直接返回（视为无改动）
    if (s_settingsHwnd && IsWindow(s_settingsHwnd)) {
        SetForegroundWindow(s_settingsHwnd);
        return FALSE;
    }

    int cx = GetSystemMetrics(SM_CXSCREEN);
    int cy = GetSystemMetrics(SM_CYSCREEN);
    int x = (cx - DIALOG_W) / 2;
    int y = (cy - DIALOG_H) / 2;

    g_changed = FALSE;

    HWND hDlg = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"SnipshotSettings", L"Settings",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        x, y, DIALOG_W, DIALOG_H,
        NULL, NULL, hInst, NULL);
    s_settingsHwnd = hDlg;

    if (!hDlg) return FALSE;

    MSG msg;
    while (IsWindow(hDlg) && GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    s_settingsHwnd = NULL;
    return g_changed;
}