#include "region.h"
#include <windowsx.h>

typedef struct {
    BOOL  selecting;
    POINT start;
    POINT current;

    /* 局部重绘缓存 */
    RECT    prev;      // 上一次失效的选区
    HDC     backDC;    // 常驻黑色背景缓冲（只建一次，避免每次鼠标移动分配全屏位图）
    HBITMAP backBmp;
    HGDIOBJ backOld;   // backDC 的原始位图
    HPEN    whitePen;  // 常驻白色描边画笔
    int     backW, backH;
} RegionState;

typedef struct {
    RECT sel;
    BOOL valid;
} RegionData;

/* 防重入：一次只允许一个选区覆盖层。
 * （RegionProc 的 rs 是跨窗口共享的 static，热键重入会互相破坏拖拽状态） */
static BOOL s_regionActive = FALSE;
static HWND s_regionHwnd = NULL;

/* 只让 "旧选区 ∪ 新选区" 外扩 2px（覆盖 2px 描边外缘）的区域失效，
 * 避免每次鼠标移动都全屏重绘（4K 下全屏位图每帧分配开销巨大） */
static void InvalidateSelRect(HWND hwnd, RegionState *rs)
{
    RECT cur;
    cur.left   = min(rs->start.x, rs->current.x);
    cur.top    = min(rs->start.y, rs->current.y);
    cur.right  = max(rs->start.x, rs->current.x);
    cur.bottom = max(rs->start.y, rs->current.y);

    RECT r;
    r.left   = min(rs->prev.left,   cur.left)   - 2;
    r.top    = min(rs->prev.top,    cur.top)    - 2;
    r.right  = max(rs->prev.right,  cur.right)  + 2;
    r.bottom = max(rs->prev.bottom, cur.bottom) + 2;

    InvalidateRect(hwnd, &r, FALSE);
    rs->prev = cur;
}

/* 确保背景缓冲存在且尺寸匹配（尺寸变化时重建） */
static BOOL EnsureRegionBack(HDC hdc, RegionState *rs, int w, int h)
{
    if (rs->backDC && rs->backBmp && rs->backW == w && rs->backH == h)
        return TRUE;

    if (rs->backDC) {
        if (rs->backBmp) SelectObject(rs->backDC, rs->backOld);
        DeleteDC(rs->backDC);
        rs->backDC = NULL;
    }
    if (rs->backBmp) { DeleteObject(rs->backBmp); rs->backBmp = NULL; }

    rs->backDC = CreateCompatibleDC(hdc);
    if (!rs->backDC) return FALSE;
    rs->backBmp = CreateCompatibleBitmap(hdc, w, h);
    if (!rs->backBmp) {
        DeleteDC(rs->backDC);
        rs->backDC = NULL;
        return FALSE;
    }
    rs->backOld = SelectObject(rs->backDC, rs->backBmp);
    rs->backW = w;
    rs->backH = h;

    RECT all = {0, 0, w, h};
    FillRect(rs->backDC, &all, GetStockObject(BLACK_BRUSH));
    return TRUE;
}

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
            InvalidateSelRect(hwnd, &rs);
            return 0;

        case WM_MOUSEMOVE:
            if (rs.selecting) {
                rs.current.x = GET_X_LPARAM(lParam);
                rs.current.y = GET_Y_LPARAM(lParam);
                InvalidateSelRect(hwnd, &rs);
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

            if (EnsureRegionBack(hdc, &rs, client.right, client.bottom)) {
                /* 背景：只把失效区域（ps.rcPaint 已裁剪）贴回黑色 */
                BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top,
                       ps.rcPaint.right - ps.rcPaint.left,
                       ps.rcPaint.bottom - ps.rcPaint.top,
                       rs.backDC, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);

                if (rs.selecting) {
                    RECT draw;
                    draw.left   = min(rs.start.x, rs.current.x);
                    draw.top    = min(rs.start.y, rs.current.y);
                    draw.right  = max(rs.start.x, rs.current.x);
                    draw.bottom = max(rs.start.y, rs.current.y);

                    if (!rs.whitePen)
                        rs.whitePen = CreatePen(PS_SOLID, 2, RGB(255,255,255));

                    HGDIOBJ oldPen   = SelectObject(hdc, rs.whitePen);
                    HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
                    Rectangle(hdc, draw.left, draw.top, draw.right, draw.bottom);
                    SelectObject(hdc, oldPen);     /* 先还原再释放，避免选中态删除失败泄漏 */
                    SelectObject(hdc, oldBrush);
                }
            }
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY:
            if (rs.backDC) {
                if (rs.backBmp) SelectObject(rs.backDC, rs.backOld);
                DeleteDC(rs.backDC);
                rs.backDC = NULL;
            }
            if (rs.backBmp) { DeleteObject(rs.backBmp); rs.backBmp = NULL; }
            if (rs.whitePen) { DeleteObject(rs.whitePen); rs.whitePen = NULL; }
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
    // 防重入：选区进行中再次触发时，聚焦已有选区窗口并返回
    if (s_regionActive) {
        if (s_regionHwnd && IsWindow(s_regionHwnd))
            SetForegroundWindow(s_regionHwnd);
        return FALSE;
    }
    s_regionActive = TRUE;

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
    s_regionHwnd = hRgn;
    if (!hRgn) {
        s_regionActive = FALSE;
        s_regionHwnd = NULL;
        return FALSE;
    }

    SetWindowLongPtrW(hRgn, GWLP_USERDATA, (LONG_PTR)&data);

    MSG msg;
    while (IsWindow(hRgn) && GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    s_regionActive = FALSE;
    s_regionHwnd = NULL;

    if (data.valid) {
        *outRect = data.sel;
        return TRUE;
    }
    return FALSE;
}