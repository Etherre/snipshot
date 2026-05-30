#include "region.h"
#include <windowsx.h>

typedef struct {
    BOOL  selecting;
    POINT start;
    POINT current;
} RegionState;

typedef struct {
    RECT sel;
    BOOL valid;
} RegionData;

LRESULT CALLBACK RegionProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static RegionState rs = {0};

    switch (msg)
    {
        case WM_CREATE:
            memset(&rs, 0, sizeof(rs));
            SetLayeredWindowAttributes(hwnd, 0, 180, LWA_ALPHA);
            return 0;

        case WM_LBUTTONDOWN:
            rs.selecting = TRUE;
            rs.start.x = GET_X_LPARAM(lParam);
            rs.start.y = GET_Y_LPARAM(lParam);
            rs.current = rs.start;
            SetCapture(hwnd);
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;

        case WM_MOUSEMOVE:
            if (rs.selecting) {
                rs.current.x = GET_X_LPARAM(lParam);
                rs.current.y = GET_Y_LPARAM(lParam);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;

        case WM_LBUTTONUP:
            if (rs.selecting) {
                ReleaseCapture();
                rs.selecting = FALSE;
                rs.current.x = GET_X_LPARAM(lParam);
                rs.current.y = GET_Y_LPARAM(lParam);

                RECT sel;
                sel.left   = min(rs.start.x, rs.current.x);
                sel.top    = min(rs.start.y, rs.current.y);
                sel.right  = max(rs.start.x, rs.current.x);
                sel.bottom = max(rs.start.y, rs.current.y);

                if ((sel.right - sel.left) >= 5 &&
                    (sel.bottom - sel.top) >= 5)
                {
                    RegionData *data = (RegionData*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
                    data->sel = sel;
                    data->valid = TRUE;
                }
                DestroyWindow(hwnd);
            }
            return 0;

        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                DestroyWindow(hwnd);
            }
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT client;
            GetClientRect(hwnd, &client);

            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBmp = CreateCompatibleBitmap(hdc, client.right, client.bottom);
            HBITMAP oldBmp = SelectObject(memDC, memBmp);

            HBRUSH black = GetStockObject(BLACK_BRUSH);
            FillRect(memDC, &client, black);

            if (rs.selecting) {
                RECT draw;
                draw.left   = min(rs.start.x, rs.current.x);
                draw.top    = min(rs.start.y, rs.current.y);
                draw.right  = max(rs.start.x, rs.current.x);
                draw.bottom = max(rs.start.y, rs.current.y);

                HBRUSH nullBrush = GetStockObject(NULL_BRUSH);
                HPEN whitePen = CreatePen(PS_SOLID, 2, RGB(255,255,255));
                SelectObject(memDC, nullBrush);
                SelectObject(memDC, whitePen);
                Rectangle(memDC, draw.left, draw.top, draw.right, draw.bottom);
                DeleteObject(whitePen);
            }

            BitBlt(hdc, 0, 0, client.right, client.bottom, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, oldBmp);
            DeleteObject(memBmp);
            DeleteDC(memDC);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY:
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

BOOL RegisterRegionClass(HINSTANCE hInst)
{
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = RegionProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"SnipshotRegion";
    wc.hCursor = LoadCursor(NULL, IDC_CROSS);
    wc.hbrBackground = NULL;
    return RegisterClassW(&wc);
}

BOOL RunRegionSelection(HINSTANCE hInst, RECT *outRect)
{
    int vx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int vy = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int vw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int vh = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    RegionData data = {{0}, FALSE};

    HWND hRgn = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        L"SnipshotRegion", NULL,
        WS_POPUP | WS_VISIBLE,
        vx, vy, vw, vh,
        NULL, NULL, hInst, NULL);
    if (!hRgn) return FALSE;

    SetWindowLongPtrW(hRgn, GWLP_USERDATA, (LONG_PTR)&data);

    MSG msg;
    while (IsWindow(hRgn) && GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (data.valid) {
        *outRect = data.sel;
        return TRUE;
    }
    return FALSE;
}