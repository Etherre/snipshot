#include "image_window.h"
#include "utils.h"
#include <windowsx.h>

// 缩放位图（从原始位图按新尺寸生成缩放后的位图）
static HBITMAP ScaleBitmap(HBITMAP orig, int origW, int origH, int newW, int newH, void **out_bits)
{
    HBITMAP scaled = CreateARGBDIB(newW, newH, out_bits);
    if (!scaled) return NULL;

    HDC screen = GetDC(NULL);
    HDC memSrc = CreateCompatibleDC(screen);
    HDC memDst = CreateCompatibleDC(screen);

    HBITMAP oldSrc = SelectObject(memSrc, orig);
    HBITMAP oldDst = SelectObject(memDst, scaled);

    SetStretchBltMode(memDst, HALFTONE);
    SetBrushOrgEx(memDst, 0, 0, NULL);
    StretchBlt(memDst, 0, 0, newW, newH,
               memSrc, 0, 0, origW, origH, SRCCOPY);

    SelectObject(memSrc, oldSrc);
    SelectObject(memDst, oldDst);
    DeleteDC(memSrc);
    DeleteDC(memDst);
    ReleaseDC(NULL, screen);
    return scaled;
}

// 分层窗口更新
static void UploadLayeredBitmap(HWND hwnd, HBITMAP bmp, BYTE alpha)
{
    BITMAP bm;
    GetObject(bmp, sizeof(bm), &bm);

    HDC screen = GetDC(NULL);
    HDC mem = CreateCompatibleDC(screen);
    HBITMAP old = SelectObject(mem, bmp);

    RECT rc;
    GetWindowRect(hwnd, &rc);
    SIZE size = { rc.right - rc.left, rc.bottom - rc.top };
    POINT src = {0, 0};
    POINT dst = { rc.left, rc.top };

    BLENDFUNCTION blend = { AC_SRC_OVER, 0, alpha, 0 };
    UpdateLayeredWindow(hwnd, screen, &dst, &size, mem, &src, 0, &blend, ULW_ALPHA);

    SelectObject(mem, old);
    DeleteDC(mem);
    ReleaseDC(NULL, screen);
}

LRESULT CALLBACK ImageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    IMAGE_WINDOW *img = (IMAGE_WINDOW*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    switch (msg)
    {
        case WM_CREATE:
        {
            CREATESTRUCTW *cs = (CREATESTRUCTW*)lParam;
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
            return 0;
        }

        case WM_LBUTTONDOWN:
            PostMessageW(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            return 0;

        case WM_MOUSEWHEEL:
        {
            if (!img) return 0;

            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            BOOL ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

            if (ctrl) {
                /* Ctrl+滚轮：调节透明度 */
                img->alpha += (delta > 0) ? 15 : -15;
                if (img->alpha < 30) img->alpha = 30;
                if (img->alpha > 255) img->alpha = 255;
                UploadLayeredBitmap(hwnd, img->bmp, (BYTE)img->alpha);
            } else {
                /* 无修饰滚轮：缩放 */
                float newScale = img->scale;
                if (delta > 0)
                    newScale *= 1.1f;
                else
                    newScale /= 1.1f;

                if (newScale < 0.1f) newScale = 0.1f;
                if (newScale > 10.0f) newScale = 10.0f;

                int newW = (int)(img->orig_width * newScale + 0.5f);
                int newH = (int)(img->orig_height * newScale + 0.5f);
                if (newW < 10) newW = 10;
                if (newH < 10) newH = 10;

                RECT rc;
                GetWindowRect(hwnd, &rc);
                int cx = (rc.left + rc.right) / 2;
                int cy = (rc.top + rc.bottom) / 2;
                int newX = cx - newW / 2;
                int newY = cy - newH / 2;

                HBITMAP scaledBmp = ScaleBitmap(img->orig_bmp, img->orig_width, img->orig_height,
                                                 newW, newH, NULL);
                if (!scaledBmp) return 0;

                if (img->bmp && img->bmp != img->orig_bmp)
                    DeleteObject(img->bmp);

                img->bmp = scaledBmp;
                img->width = newW;
                img->height = newH;
                img->scale = newScale;

                SetWindowPos(hwnd, NULL, newX, newY, newW, newH,
                             SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);
                UploadLayeredBitmap(hwnd, img->bmp, (BYTE)img->alpha);
            }
            return 0;
        }

        case WM_RBUTTONUP:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            if (img) {
                if (img->bmp && img->bmp != img->orig_bmp)
                    DeleteObject(img->bmp);
                if (img->orig_bmp)
                    DeleteObject(img->orig_bmp);
                HeapFree(GetProcessHeap(), 0, img);
            }
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

BOOL RegisterImageClass(HINSTANCE hInst)
{
    WNDCLASSW wc = {0};
    wc.hInstance = hInst;
    wc.lpfnWndProc = ImageProc;
    wc.lpszClassName = L"SnipshotImage";
    return RegisterClassW(&wc);
}

HWND CreateImageWindow(HINSTANCE hInst, int x, int y, int w, int h, HBITMAP origBmp, void *origBits)
{
    IMAGE_WINDOW *img = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IMAGE_WINDOW));
    if (!img) return NULL;

    img->alpha = 255;
    img->orig_bmp = origBmp;
    img->orig_bits = origBits;
    img->orig_width = w;
    img->orig_height = h;
    img->scale = 1.0f;
    img->bmp = img->orig_bmp;
    img->width = w;
    img->height = h;

    HWND hOverlay = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"SnipshotImage", NULL,
        WS_POPUP,
        x, y, w, h,
        NULL, NULL, hInst, img);
    if (!hOverlay) {
        DeleteObject(img->orig_bmp);
        HeapFree(GetProcessHeap(), 0, img);
        return NULL;
    }

    UploadLayeredBitmap(hOverlay, img->bmp, 255);
    ShowWindow(hOverlay, SW_SHOW);
    return hOverlay;
}