#include "drawing.h"
#include "utils.h"
#include <windowsx.h>

/* ── 默认值 ── */
#define PEN_COLOR    RGB(255, 0, 0)
#define PEN_SIZE     3
#define PEN_MIN      1
#define PEN_MAX      20

/* ── 工具栏布局 ── */
#define BAR_W         340
#define BAR_H         40
#define CLR_SIZE      24
#define CLR_GAP       4
#define CLR_COUNT     6
#define CLR_Y         8
#define CLR_X0        8

#define INDICATOR_X   (CLR_X0 + CLR_COUNT * (CLR_SIZE + CLR_GAP) + 8)
#define CLEAR_X       (INDICATOR_X + 60 + 8)
#define CLEAR_W       56
#define CLOSE_X       (CLEAR_X + CLEAR_W + 4)
#define CLOSE_W       24
#define BTN_Y         6
#define BTN_H         28

/* ── 色板 ── */
static const COLORREF kPalette[CLR_COUNT] = {
    RGB(255, 0, 0),    // 红
    RGB(255, 165, 0),  // 橙
    RGB(0, 180, 0),    // 绿
    RGB(0, 120, 255),  // 蓝
    RGB(255, 255, 255),// 白
    RGB(0, 0, 0),      // 黑
};

/* ── 定时器 ID ── */
#define IDT_ALT_CHECK 1

/* ── 全局画板状态 ── */
static struct {
    HBITMAP  canvas_bmp;
    BYTE    *canvas_bits;
    int      w, h;

    COLORREF pen_color;
    int      pen_size;

    BOOL     drawing;
    BOOL     erasing;
    int      last_x, last_y;

    HWND     hwndCanvas;
    HWND     hwndToolbar;
    DWORD    last_upload;

    // 持久化 DC 和缓存的 UpdateLayeredWindow 参数，避免每帧创建/销毁
    HDC      hdcMem;
    SIZE     ulwSize;
    POINT    ulwDst;
    POINT    ulwSrc;
} g_draw;

/* ── DIB 像素绘制 ── */

// 抗锯齿画圆：在圆边缘 ~1px 范围内做平滑 alpha 过渡，消除毛刺
static void DrawDot(int cx, int cy, int r, COLORREF color, BYTE alpha)
{
    BYTE r8 = GetRValue(color);
    BYTE g8 = GetGValue(color);
    BYTE b8 = GetBValue(color);

    // 扩大采样范围以包含抗锯齿过渡带
    int x0 = max(0, cx - r - 1);
    int y0 = max(0, cy - r - 1);
    int x1 = min(g_draw.w - 1, cx + r + 1);
    int y1 = min(g_draw.h - 1, cy + r + 1);

    int stride = g_draw.w * 4;
    int rr = r * r;

    // 过渡带宽度 (d² 空间): r<=1 不需要，大 r 自动缩放到 ~1px
    int band = (r <= 1) ? 0 : r;

    for (int y = y0; y <= y1; y++) {
        BYTE *row = g_draw.canvas_bits + (size_t)y * stride;
        for (int x = x0; x <= x1; x++) {
            int dx = x - cx, dy = y - cy;
            int d2 = dx * dx + dy * dy;

            if (d2 <= rr - band) {
                // 圆心深处：完全不透明
                BYTE *px = row + (size_t)x * 4;
                if (alpha == 255 || alpha >= px[3]) {
                    px[0] = b8; px[1] = g8; px[2] = r8; px[3] = alpha;
                }
            } else if (band > 0 && d2 < rr + band + 1) {
                // 过渡带：根据 d² 平滑插值 alpha
                int pos = d2 - (rr - band);          // [0, 2*band+1)
                int range = 2 * band + 1;
                BYTE aa = (BYTE)((unsigned)(range - pos) * 255 / range);

                BYTE *px = row + (size_t)x * 4;
                if (alpha == 255) {
                    // 画笔：用抗锯齿 alpha 覆盖
                    if (aa >= px[3]) {
                        px[0] = b8; px[1] = g8; px[2] = r8; px[3] = aa;
                    }
                } else {
                    // 橡皮擦 (alpha=1)：硬边擦除，不用柔和过渡
                    if (aa >= 128) {
                        px[3] = 1;
                    }
                }
            }
        }
    }
}

static void DrawLine(int x0, int y0, int x1, int y1,
                     int r, COLORREF color, BYTE alpha)
{
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    for (;;) {
        DrawDot(x0, y0, r, color, alpha);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

static void ClearCanvas(void)
{
    DWORD *p = (DWORD*)g_draw.canvas_bits;
    DWORD *end = p + (size_t)g_draw.w * g_draw.h;
    while (p < end) *p++ = 0x01000000;  // BGRA = (0,0,0,1)
}

/* ── 切换鼠标穿透 ── */

static void SetPassthrough(BOOL enable)
{
    LONG_PTR ex = GetWindowLongPtrW(g_draw.hwndCanvas, GWL_EXSTYLE);
    if (enable)
        ex |= WS_EX_TRANSPARENT;
    else
        ex &= ~WS_EX_TRANSPARENT;
    SetWindowLongPtrW(g_draw.hwndCanvas, GWL_EXSTYLE, ex);
    SetWindowPos(g_draw.hwndCanvas, NULL, 0,0,0,0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                 SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

/* ── 分层窗口上传 ── */

// 使用持久化 DC + 缓存参数，避免每帧 CreateCompatibleDC/GetWindowRect
static void UploadCanvas(void)
{
    HDC screen = GetDC(NULL);
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    UpdateLayeredWindow(g_draw.hwndCanvas, screen,
                        &g_draw.ulwDst, &g_draw.ulwSize,
                        g_draw.hdcMem, &g_draw.ulwSrc,
                        0, &blend, ULW_ALPHA);
    ReleaseDC(NULL, screen);
}

/* ── 工具栏 ── */

// 获取颜色按钮矩形（0-based index）
static RECT ColorRect(int i)
{
    int x = CLR_X0 + i * (CLR_SIZE + CLR_GAP);
    RECT r = {x, CLR_Y, x + CLR_SIZE, CLR_Y + CLR_SIZE};
    return r;
}

// 点击测试：-1=无, 0-5=颜色, 6=Clear, 7=Close, -2=拖拽区
static int HitTestToolbar(int mx, int my)
{
    for (int i = 0; i < CLR_COUNT; i++) {
        RECT r = ColorRect(i);
        POINT pt = {mx, my};
        if (PtInRect(&r, pt))
            return i;
    }
    RECT clear = {CLEAR_X, BTN_Y, CLEAR_X + CLEAR_W, BTN_Y + BTN_H};
    { POINT pt = {mx, my}; if (PtInRect(&clear, pt)) return 6; }

    RECT close = {CLOSE_X, BTN_Y, CLOSE_X + CLOSE_W, BTN_Y + BTN_H};
    { POINT pt = {mx, my}; if (PtInRect(&close, pt)) return 7; }

    return -1;
}

LRESULT CALLBACK ToolbarProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT client;
            GetClientRect(hwnd, &client);

            // 背景
            HBRUSH bg = CreateSolidBrush(RGB(45, 45, 48));
            FillRect(hdc, &client, bg);
            DeleteObject(bg);

            // ── 颜色按钮 ──
            for (int i = 0; i < CLR_COUNT; i++) {
                RECT r = ColorRect(i);
                HBRUSH clrBr = CreateSolidBrush(kPalette[i]);
                FillRect(hdc, &r, clrBr);
                DeleteObject(clrBr);

                // 边框：当前颜色加亮边框，其他暗边框
                BOOL sel = (kPalette[i] == g_draw.pen_color);
                HPEN border = CreatePen(PS_SOLID, sel ? 2 : 1,
                                        sel ? RGB(255,255,255) : RGB(80,80,84));
                HBRUSH nullBr = GetStockObject(NULL_BRUSH);
                SelectObject(hdc, border);
                SelectObject(hdc, nullBr);
                Rectangle(hdc, r.left, r.top, r.right, r.bottom);
                DeleteObject(border);
            }

            // ── 画笔状态指示：色点 + 粗细 ──
            {
                int cx = INDICATOR_X + 10;
                int cy = BAR_H / 2;
                int dotR = min(g_draw.pen_size + 1, 8);

                // 色点
                HBRUSH dotBr = CreateSolidBrush(g_draw.pen_color);
                HPEN dotPen = CreatePen(PS_SOLID, 1, RGB(160,160,164));
                SelectObject(hdc, dotBr);
                SelectObject(hdc, dotPen);
                Ellipse(hdc, cx - dotR, cy - dotR, cx + dotR + 1, cy + dotR + 1);
                DeleteObject(dotPen);
                DeleteObject(dotBr);

                // 粗细文本
                wchar_t buf[16];
                wsprintfW(buf, L"%dpx", g_draw.pen_size);
                RECT tr = {cx + 14, 0, INDICATOR_X + 58, BAR_H};
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, RGB(200, 200, 204));
                DrawTextW(hdc, buf, -1, &tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            }

            // ── Clear 按钮 ──
            {
                RECT r = {CLEAR_X, BTN_Y, CLEAR_X + CLEAR_W, BTN_Y + BTN_H};
                HBRUSH face = CreateSolidBrush(RGB(62, 62, 66));
                FillRect(hdc, &r, face);
                DeleteObject(face);

                HPEN border = CreatePen(PS_SOLID, 1, RGB(100, 100, 104));
                HBRUSH nullBr = GetStockObject(NULL_BRUSH);
                SelectObject(hdc, border);
                SelectObject(hdc, nullBr);
                Rectangle(hdc, r.left, r.top, r.right, r.bottom);
                DeleteObject(border);

                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, RGB(240, 240, 240));
                DrawTextW(hdc, L"Clear", -1, &r,
                          DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }

            // ── Close 按钮 ──
            {
                RECT r = {CLOSE_X, BTN_Y, CLOSE_X + CLOSE_W, BTN_Y + BTN_H};
                HBRUSH face = CreateSolidBrush(RGB(62, 62, 66));
                FillRect(hdc, &r, face);
                DeleteObject(face);

                HPEN border = CreatePen(PS_SOLID, 1, RGB(100, 100, 104));
                HBRUSH nullBr = GetStockObject(NULL_BRUSH);
                SelectObject(hdc, border);
                SelectObject(hdc, nullBr);
                Rectangle(hdc, r.left, r.top, r.right, r.bottom);
                DeleteObject(border);

                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, RGB(240, 240, 240));
                DrawTextW(hdc, L"X", -1, &r,
                          DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN:
        {
            int mx = GET_X_LPARAM(lParam);
            int my = GET_Y_LPARAM(lParam);
            int btn = HitTestToolbar(mx, my);

            if (btn >= 0 && btn < CLR_COUNT) {
                g_draw.pen_color = kPalette[btn];
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (btn == 6) {  // Clear
                ClearCanvas();
                UploadCanvas();
                return 0;
            }
            if (btn == 7) {  // Close
                DestroyWindow(g_draw.hwndCanvas);
                return 0;
            }
            // 拖拽移动
            PostMessageW(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            return 0;
        }

        case WM_DESTROY:
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

/* ── 画布窗口过程 ── */

LRESULT CALLBACK CanvasProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CREATE:
            return 0;

        // ── 左键 = 画笔 ──
        case WM_LBUTTONDOWN:
        {
            g_draw.drawing = TRUE;
            g_draw.erasing = FALSE;
            g_draw.last_x = GET_X_LPARAM(lParam);
            g_draw.last_y = GET_Y_LPARAM(lParam);
            SetCapture(hwnd);

            DrawDot(g_draw.last_x, g_draw.last_y,
                    g_draw.pen_size, g_draw.pen_color, 255);
            UploadCanvas();
            g_draw.last_upload = GetTickCount();
            return 0;
        }

        // ── 右键按住 = 橡皮擦，松开自动回到空闲 ──
        case WM_RBUTTONDOWN:
        {
            g_draw.drawing = TRUE;
            g_draw.erasing = TRUE;
            g_draw.last_x = GET_X_LPARAM(lParam);
            g_draw.last_y = GET_Y_LPARAM(lParam);
            SetCapture(hwnd);

            DrawDot(g_draw.last_x, g_draw.last_y,
                    g_draw.pen_size, RGB(0,0,0), 1);
            UploadCanvas();
            g_draw.last_upload = GetTickCount();
            return 0;
        }

        case WM_MOUSEMOVE:
            if (!g_draw.drawing) return 0;

            {
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);
                int      r = g_draw.pen_size;
                COLORREF c = g_draw.erasing ? RGB(0,0,0) : g_draw.pen_color;
                BYTE    a = g_draw.erasing ? 1 : 255;

                DrawLine(g_draw.last_x, g_draw.last_y, x, y, r, c, a);
                g_draw.last_x = x;
                g_draw.last_y = y;

                DWORD now = GetTickCount();
                if (now - g_draw.last_upload >= 16) {
                    UploadCanvas();
                    g_draw.last_upload = now;
                }
            }
            return 0;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            if (g_draw.drawing) {
                ReleaseCapture();
                g_draw.drawing = FALSE;
                UploadCanvas();
            }
            return 0;

        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
            return 0;

        // ── 滚轮：调节画笔/橡皮粗细（共用同一尺寸）──
        case WM_MOUSEWHEEL:
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            g_draw.pen_size += (delta > 0) ? 1 : -1;
            if (g_draw.pen_size < PEN_MIN) g_draw.pen_size = PEN_MIN;
            if (g_draw.pen_size > PEN_MAX) g_draw.pen_size = PEN_MAX;
            InvalidateRect(g_draw.hwndToolbar, NULL, FALSE);
            return 0;
        }

        // ── Alt 穿透 ──
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
            if (wParam == VK_MENU) {
                SetPassthrough(TRUE);
                SetTimer(hwnd, IDT_ALT_CHECK, 50, NULL);
                return 0;
            }
            if (wParam == VK_ESCAPE) {
                DestroyWindow(hwnd);
                return 0;
            }
            return 0;

        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (wParam == VK_MENU) {
                SetPassthrough(FALSE);
                KillTimer(hwnd, IDT_ALT_CHECK);
                if (GetFocus() != hwnd)
                    SetFocus(hwnd);
                return 0;
            }
            return 0;

        case WM_TIMER:
            if (wParam == IDT_ALT_CHECK) {
                if (!(GetAsyncKeyState(VK_MENU) & 0x8000)) {
                    SetPassthrough(FALSE);
                    KillTimer(hwnd, IDT_ALT_CHECK);
                    if (GetFocus() != hwnd)
                        SetFocus(hwnd);
                }
                return 0;
            }
            return 0;

        case WM_DESTROY:
            KillTimer(hwnd, IDT_ALT_CHECK);
            SetPassthrough(FALSE);
            if (g_draw.hdcMem) {
                DeleteDC(g_draw.hdcMem);
                g_draw.hdcMem = NULL;
            }
            if (g_draw.canvas_bmp) {
                DeleteObject(g_draw.canvas_bmp);
                g_draw.canvas_bmp = NULL;
                g_draw.canvas_bits = NULL;
            }
            if (g_draw.hwndToolbar && IsWindow(g_draw.hwndToolbar))
                DestroyWindow(g_draw.hwndToolbar);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

/* ── 公开接口 ── */

BOOL RegisterDrawingClass(HINSTANCE hInst)
{
    WNDCLASSW wc = {0};

    wc.hInstance     = hInst;
    wc.lpfnWndProc   = CanvasProc;
    wc.lpszClassName = L"SnipshotDrawing";
    wc.hCursor       = LoadCursor(NULL, IDC_CROSS);
    wc.hbrBackground = NULL;
    if (!RegisterClassW(&wc)) return FALSE;

    wc.lpfnWndProc   = ToolbarProc;
    wc.lpszClassName = L"SnipshotDrawingBar";
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    if (!RegisterClassW(&wc)) return FALSE;

    return TRUE;
}

void RunDrawingMode(HINSTANCE hInst)
{
    memset(&g_draw, 0, sizeof(g_draw));
    g_draw.pen_color = PEN_COLOR;
    g_draw.pen_size  = PEN_SIZE;

    int vx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int vy = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int vw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int vh = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    void *bits = NULL;
    g_draw.canvas_bmp = CreateARGBDIB(vw, vh, &bits);
    if (!g_draw.canvas_bmp) return;
    g_draw.canvas_bits = (BYTE*)bits;
    g_draw.w = vw;
    g_draw.h = vh;
    ClearCanvas();

    g_draw.hwndCanvas = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"SnipshotDrawing", NULL,
        WS_POPUP,
        vx, vy, vw, vh,
        NULL, NULL, hInst, NULL);
    if (!g_draw.hwndCanvas) {
        DeleteObject(g_draw.canvas_bmp);
        g_draw.canvas_bmp = NULL;
        return;
    }
    // 创建持久化内存 DC 并缓存在 UploadLayeredWindow 参数中
    {
        HDC screen = GetDC(NULL);
        g_draw.hdcMem = CreateCompatibleDC(screen);
        ReleaseDC(NULL, screen);
        SelectObject(g_draw.hdcMem, g_draw.canvas_bmp);

        g_draw.ulwSrc.x = 0;
        g_draw.ulwSrc.y = 0;
        g_draw.ulwDst.x = vx;
        g_draw.ulwDst.y = vy;
        g_draw.ulwSize.cx = vw;
        g_draw.ulwSize.cy = vh;
    }

    UploadCanvas();
    ShowWindow(g_draw.hwndCanvas, SW_SHOW);
    SetFocus(g_draw.hwndCanvas);

    int barX = GetSystemMetrics(SM_CXSCREEN) / 2 - BAR_W / 2;
    int barY = 50;
    g_draw.hwndToolbar = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        L"SnipshotDrawingBar", NULL,
        WS_POPUP | WS_VISIBLE,
        barX, barY, BAR_W, BAR_H,
        g_draw.hwndCanvas, NULL, hInst, NULL);

    MSG msg;
    while (IsWindow(g_draw.hwndCanvas) && GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}