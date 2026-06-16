#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <stdint.h>
#include <stdbool.h>
#include "utils.h"
#include "region.h"
#include "image_window.h"
#include "drawing.h"
#include "about.h"

/* 消息与 ID */
#define WMAPP_NOTIFYCALLBACK  (WM_APP + 1)
#define IDM_CAPTURE_PIN       1001
#define IDM_CAPTURE_PIN_COPY  1004
#define IDM_CAPTURE_COPY      1005
#define IDM_AUTORUN           1002
#define IDM_EXIT              1003
#define IDHOT_PIN             2001
#define IDHOT_PIN_COPY        2002
#define IDHOT_COPY            2003
#define IDHOT_DRAWING         2004
#define IDI_APPICON           101
#define IDM_DRAWING           1006
#define IDM_ABOUT             1007

typedef enum {
    MODE_PIN,        // 截图并贴图
    MODE_PIN_COPY,   // 截图贴图并复制
    MODE_COPY,       // 截图复制（不贴图）
} CaptureMode;

/* 全局变量 */
HINSTANCE g_hInst;
HWND g_hwndMain;
NOTIFYICONDATAW g_nid = {0};

static void DoCapture(CaptureMode mode)
{
    RECT sel;
    if (!RunRegionSelection(g_hInst, &sel))
        return;

    int vx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int vy = GetSystemMetrics(SM_YVIRTUALSCREEN);

    int x = sel.left + vx;
    int y = sel.top + vy;
    int w = sel.right - sel.left;
    int h = sel.bottom - sel.top;

    HBITMAP origBmp = CaptureScreenRect(x, y, w, h);
    if (!origBmp) return;

    if (mode == MODE_PIN_COPY || mode == MODE_COPY)
        CopyBitmapToClipboard(origBmp);

    if (mode == MODE_PIN || mode == MODE_PIN_COPY)
        CreateImageWindow(g_hInst, x, y, w, h, origBmp);
    else
        DeleteObject(origBmp);
}

static void ShowTrayMenu(HWND hwnd)
{
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, IDM_CAPTURE_PIN,      L"截图贴图\tCtrl+Shift+S");
    AppendMenuW(menu, MF_STRING, IDM_CAPTURE_PIN_COPY, L"截图贴图并复制\tCtrl+Shift+A");
    AppendMenuW(menu, MF_STRING, IDM_CAPTURE_COPY,     L"截图复制\tCtrl+Shift+D");
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING, IDM_DRAWING,         L"屏幕画板\tCtrl+Shift+B");

    UINT check = IsAutoRunEnabled() ? MF_CHECKED : MF_UNCHECKED;
    AppendMenuW(menu, MF_STRING | check, IDM_AUTORUN, L"开机自启");
    AppendMenuW(menu, MF_STRING, IDM_ABOUT,           L"关于");

    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING, IDM_EXIT, L"退出");

    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(hwnd);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
    DestroyMenu(menu);
}

LRESULT CALLBACK MainProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_HOTKEY:
            switch (LOWORD(wParam)) {
                case IDHOT_PIN:      DoCapture(MODE_PIN);      break;
                case IDHOT_PIN_COPY: DoCapture(MODE_PIN_COPY); break;
                case IDHOT_COPY:     DoCapture(MODE_COPY);     break;
                case IDHOT_DRAWING:  RunDrawingMode(g_hInst); break;
            }
            return 0;

        case WMAPP_NOTIFYCALLBACK:
            if (lParam == WM_RBUTTONUP)
                ShowTrayMenu(hwnd);
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDM_CAPTURE_PIN:      DoCapture(MODE_PIN);      break;
                case IDM_CAPTURE_PIN_COPY: DoCapture(MODE_PIN_COPY); break;
                case IDM_CAPTURE_COPY:     DoCapture(MODE_COPY);     break;
                case IDM_DRAWING:          RunDrawingMode(g_hInst); break;
                case IDM_ABOUT:            ShowAboutDialog(g_hInst); break;
                case IDM_AUTORUN:
                    SetAutoRun(!IsAutoRunEnabled());
                    break;
                case IDM_EXIT:
                    DestroyWindow(hwnd);
                    break;
            }
            return 0;

        case WM_DESTROY:
            Shell_NotifyIconW(NIM_DELETE, &g_nid);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
{
    (void)hPrev; (void)lpCmd; (void)nShow;

    HANDLE hMutex = CreateMutexW(NULL, TRUE, L"SnipshotSingleInstance");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        if (hMutex) { ReleaseMutex(hMutex); CloseHandle(hMutex); }
        return 0;
    }

    EnableDpi();
    RefreshAutoRunPath();
    g_hInst = hInst;

    RegisterRegionClass(hInst);
    RegisterImageClass(hInst);
    RegisterDrawingClass(hInst);
    RegisterAboutClass(hInst);

    WNDCLASSW wc = {0};
    wc.hInstance = hInst;
    wc.lpfnWndProc = MainProc;
    wc.lpszClassName = L"SnipshotMain";
    RegisterClassW(&wc);

    g_hwndMain = CreateWindowExW(0, L"SnipshotMain", NULL, 0, 0,0,0,0, NULL,NULL, hInst, NULL);

    HICON icon = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_APPICON), IMAGE_ICON, 0,0, LR_DEFAULTSIZE);
    if (!icon) icon = LoadIcon(NULL, IDI_APPLICATION);

    g_nid.cbSize = sizeof(g_nid);
    g_nid.hWnd = g_hwndMain;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WMAPP_NOTIFYCALLBACK;
    g_nid.hIcon = icon;
    lstrcpyW(g_nid.szTip, L"Snipshot");
    Shell_NotifyIconW(NIM_ADD, &g_nid);

    RegisterHotKey(g_hwndMain, IDHOT_PIN,      MOD_CONTROL | MOD_SHIFT, 'S');
    RegisterHotKey(g_hwndMain, IDHOT_PIN_COPY, MOD_CONTROL | MOD_SHIFT, 'A');
    RegisterHotKey(g_hwndMain, IDHOT_COPY,     MOD_CONTROL | MOD_SHIFT, 'D');
    RegisterHotKey(g_hwndMain, IDHOT_DRAWING,  MOD_CONTROL | MOD_SHIFT, 'B');

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    UnregisterHotKey(g_hwndMain, IDHOT_PIN);
    UnregisterHotKey(g_hwndMain, IDHOT_PIN_COPY);
    UnregisterHotKey(g_hwndMain, IDHOT_COPY);
    UnregisterHotKey(g_hwndMain, IDHOT_DRAWING);
    if (hMutex) { ReleaseMutex(hMutex); CloseHandle(hMutex); }
    return 0;
}