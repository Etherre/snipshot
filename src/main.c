#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <stdint.h>
#include <stdbool.h>
#include "utils.h"
#include "region.h"
#include "image_window.h"

/* 消息与 ID */
#define WMAPP_NOTIFYCALLBACK  (WM_APP + 1)
#define IDM_CAPTURE_REGION    1001
#define IDM_AUTORUN           1002
#define IDM_EXIT              1003
#define IDHOT_CAPTURE         2001
#define IDI_APPICON           101

/* 全局变量 */
HINSTANCE g_hInst;
HWND g_hwndMain;
NOTIFYICONDATAW g_nid = {0};

static void DoCapture(void)
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

    void *origBits;
    HBITMAP origBmp = CaptureScreenRect(x, y, w, h, &origBits);
    if (!origBmp) return;

    CreateImageWindow(g_hInst, x, y, w, h, origBmp, origBits);
}

LRESULT CALLBACK MainProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_HOTKEY:
            DoCapture();
            return 0;

        case WMAPP_NOTIFYCALLBACK:
            if (lParam == WM_RBUTTONUP) {
                HMENU menu = CreatePopupMenu();
                AppendMenuW(menu, MF_STRING, IDM_CAPTURE_REGION, L"选区截图并贴图\tCtrl+Shift+S");

                UINT check = IsAutoRunEnabled() ? MF_CHECKED : MF_UNCHECKED;
                AppendMenuW(menu, MF_STRING | check, IDM_AUTORUN, L"开机自启");

                AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
                AppendMenuW(menu, MF_STRING, IDM_EXIT, L"退出");

                POINT pt;
                GetCursorPos(&pt);
                SetForegroundWindow(hwnd);
                TrackPopupMenu(menu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu(menu);
            }
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDM_CAPTURE_REGION:
                    DoCapture();
                    break;
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

    // 注册窗口类
    RegisterRegionClass(hInst);
    RegisterImageClass(hInst);

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

    RegisterHotKey(g_hwndMain, IDHOT_CAPTURE, MOD_CONTROL | MOD_SHIFT, 'S');

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    UnregisterHotKey(g_hwndMain, IDHOT_CAPTURE);
    if (hMutex) { ReleaseMutex(hMutex); CloseHandle(hMutex); }
    return 0;
}