/* Stub implementations of the Win32 APIs referenced by drawing.c / hotkey.c.
 * Pure-logic tests never exercise window procedures, so most stubs are inert. */
#include "windows.h"
#include <stdarg.h>
#include <stdio.h>

HDC GetDC(HWND h){ (void)h; return (HDC)1; }
int ReleaseDC(HWND h, HDC d){ (void)h; (void)d; return 1; }
HDC CreateCompatibleDC(HDC d){ (void)d; return (HDC)1; }
BOOL DeleteDC(HDC d){ (void)d; return TRUE; }
HBITMAP CreateCompatibleBitmap(HDC d, int w, int h){ (void)d; (void)w; (void)h; return (HBITMAP)1; }
HBITMAP CreateARGBDIB(int w, int h, void **bits){ (void)w; (void)h; if (bits) *bits = NULL; return NULL; }
BOOL UpdateLayeredWindow(HWND h, HDC a, POINT *b, SIZE *c, HDC d, POINT *e, COLORREF f, BLENDFUNCTION *g, DWORD i){
    (void)h;(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)i; return TRUE;
}
LONG_PTR GetWindowLongPtrW(HWND h, int i){ (void)h; (void)i; return 0; }
LONG_PTR SetWindowLongPtrW(HWND h, int i, LONG_PTR v){ (void)h; (void)i; (void)v; return 0; }
BOOL SetWindowPos(HWND h, HWND a, int x, int y, int w, int z, UINT f){
    (void)h;(void)a;(void)x;(void)y;(void)w;(void)z;(void)f; return TRUE;
}
int FillRect(HDC h, const RECT *r, HBRUSH b){ (void)h; (void)r; (void)b; return 1; }
HBRUSH CreateSolidBrush(COLORREF c){ (void)c; return (HBRUSH)1; }
BOOL DeleteObject(HGDIOBJ o){ (void)o; return TRUE; }
HPEN CreatePen(int s, int w, COLORREF c){ (void)s; (void)w; (void)c; return (HPEN)1; }
HGDIOBJ SelectObject(HDC h, HGDIOBJ o){ (void)h; return o; }
BOOL Rectangle(HDC h, int a, int b, int c, int d){ (void)h;(void)a;(void)b;(void)c;(void)d; return TRUE; }
HDC BeginPaint(HWND h, PAINTSTRUCT *p){ (void)h; if (p) memset(p, 0, sizeof(*p)); return (HDC)1; }
BOOL GetClientRect(HWND h, RECT *r){ (void)h; (void)r; return FALSE; }
BOOL EndPaint(HWND h, const PAINTSTRUCT *p){ (void)h; (void)p; return TRUE; }
int DrawTextW(HDC h, LPCWSTR s, int n, RECT *r, UINT f){ (void)h;(void)s;(void)n;(void)r;(void)f; return 0; }
int SetBkMode(HDC h, int m){ (void)h; (void)m; return 0; }
COLORREF SetTextColor(HDC h, COLORREF c){ (void)h; return c; }
BOOL Ellipse(HDC h, int a, int b, int c, int d){ (void)h;(void)a;(void)b;(void)c;(void)d; return TRUE; }
int wsprintfW(LPWSTR out, LPCWSTR fmt, ...){
    va_list ap; va_start(ap, fmt);
    int r = vswprintf(out, 4096, fmt, ap);
    va_end(ap);
    return r;
}
static BOOL fake_inv_called = FALSE;
static RECT fake_inv_rect = {0};
void fake_clear_invalidate(void){ fake_inv_called = FALSE; memset(&fake_inv_rect, 0, sizeof(fake_inv_rect)); }
BOOL fake_last_invalidate(RECT *out){
    if (out && fake_inv_called) *out = fake_inv_rect;
    return fake_inv_called;
}
BOOL InvalidateRect(HWND h, const RECT *r, BOOL e){
    (void)h; (void)e;
    fake_inv_called = TRUE;
    if (r) fake_inv_rect = *r;
    return TRUE;
}
BOOL DestroyWindow(HWND h){ (void)h; return TRUE; }
BOOL PostMessageW(HWND h, UINT m, WPARAM w, LPARAM l){ (void)h;(void)m;(void)w;(void)l; return TRUE; }
DWORD GetTickCount(void){ return 0; }
HWND SetCapture(HWND h){ return h; }
BOOL ReleaseCapture(void){ return TRUE; }
SHORT GetAsyncKeyState(int v){ (void)v; return 0; }
UINT_PTR SetTimer(HWND h, UINT_PTR id, UINT ms, TIMERPROC p){ (void)h;(void)ms;(void)p; return id; }
BOOL KillTimer(HWND h, UINT_PTR id){ (void)h; (void)id; return TRUE; }
HWND SetFocus(HWND h){ return h; }
HWND GetFocus(void){ return NULL; }
BOOL ShowWindow(HWND h, int c){ (void)h; (void)c; return TRUE; }
ATOM RegisterClassW(const WNDCLASSW *w){ (void)w; return 1; }
HCURSOR LoadCursor(HINSTANCE h, LPCWSTR n){ (void)h; (void)n; return NULL; }
int GetSystemMetrics(int i){ (void)i; return 1920; }
HWND CreateWindowExW(DWORD e, LPCWSTR c, LPCWSTR t, DWORD s, int x, int y, int w, int h,
                     HWND p, HMENU m, HINSTANCE i, LPVOID l){
    (void)e;(void)c;(void)t;(void)s;(void)x;(void)y;(void)w;(void)h;(void)p;(void)m;(void)i;(void)l;
    return NULL;
}
BOOL GetMessageW(MSG *m, HWND h, UINT a, UINT b){ (void)m;(void)h;(void)a;(void)b; return FALSE; }
BOOL TranslateMessage(const MSG *m){ (void)m; return FALSE; }
LRESULT DispatchMessageW(const MSG *m){ (void)m; return 0; }
BOOL IsWindow(HWND h){ return h != NULL; }
LRESULT DefWindowProcW(HWND h, UINT m, WPARAM w, LPARAM l){ (void)h;(void)m;(void)w;(void)l; return 0; }
HGDIOBJ GetStockObject(int i){ return (HGDIOBJ)(intptr_t)(i + 1); }
BOOL SetLayeredWindowAttributes(HWND h, COLORREF c, BYTE b, DWORD f){
    (void)h; (void)c; (void)b; (void)f; return TRUE;
}
BOOL BitBlt(HDC h, int a, int b, int c, int d, HDC s, int e, int f, DWORD r){
    (void)h;(void)a;(void)b;(void)c;(void)d;(void)s;(void)e;(void)f;(void)r;
    return TRUE;
}
HFONT CreateFontW(int a, int b, int c, int d, int e, DWORD f, DWORD g, DWORD h,
                  DWORD i, DWORD j, DWORD k, DWORD l, DWORD m, LPCWSTR n){
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;
    (void)i;(void)j;(void)k;(void)l;(void)m;(void)n;
    return (HFONT)1;
}
BOOL MoveToEx(HDC h, int x, int y, POINT *p){ (void)h;(void)x;(void)y;(void)p; return TRUE; }
BOOL LineTo(HDC h, int x, int y){ (void)h;(void)x;(void)y; return TRUE; }
HINSTANCE ShellExecuteW(HWND h, LPCWSTR a, LPCWSTR b, LPCWSTR c, LPCWSTR d, INT e){
    (void)h;(void)a;(void)b;(void)c;(void)d;(void)e;
    return (HINSTANCE)1;
}
static HWND fake_fg = NULL;
void fake_clear_foreground(void){ fake_fg = NULL; }
HWND fake_last_foreground(void){ return fake_fg; }
BOOL SetForegroundWindow(HWND h){ fake_fg = h; return TRUE; }
BOOL PtInRect(const RECT *r, POINT p){
    return p.x >= r->left && p.x < r->right && p.y >= r->top && p.y < r->bottom;
}
UINT MapVirtualKeyW(UINT c, UINT t){ (void)t; return c; }

/* GetKeyNameTextW stub: mode 0 → empty (exercises FormatHotkey fallbacks);
 * mode 1 → returns the key name for letter/F-key/arrow keys. */
static int g_keynames = 0;
void fake_set_keynames(int on){ g_keynames = on; }
int GetKeyNameTextW(LONG l, LPWSTR buf, int n){
    if (buf && n > 0) buf[0] = 0;
    if (!g_keynames) return 0;
    unsigned vk = (unsigned)(l >> 16) & 0xFF; /* 剥掉 0x100 扩展键标志 */
    if (vk >= 'A' && vk <= 'Z') { if (n > 1){ buf[0] = (wchar_t)vk; buf[1] = 0; } return 1; }
    if (vk >= VK_F1 && vk <= VK_F24){ swprintf(buf, (size_t)n, L"F%u", vk - VK_F1 + 1); return 1; }
    static const struct { UINT vk; const wchar_t *n; } names[] = {
        { VK_LEFT, L"Left" }, { VK_RIGHT, L"Right" }, { VK_UP, L"Up" }, { VK_DOWN, L"Down" },
    };
    for (size_t i = 0; i < sizeof(names)/sizeof(names[0]); i++)
        if (names[i].vk == vk){ if (n > 0) wcsncpy(buf, names[i].n, (size_t)n - 1), buf[n-1] = 0; return 1; }
    return 0;
}

/* Tiny in-memory fake for the HKCU\Software\Snipshot\Hotkeys values. */
#define FAKE_MAX 32
static struct { wchar_t name[32]; DWORD val; int present; } fake[FAKE_MAX];
void fake_registry_clear(void){ memset(fake, 0, sizeof(fake)); }
int fake_registry_count(void){ int c = 0; for (int i = 0; i < FAKE_MAX; i++) if (fake[i].present) c++; return c; }
static int fake_find(const wchar_t *n){
    for (int i = 0; i < FAKE_MAX; i++) if (fake[i].present && wcscmp(fake[i].name, n) == 0) return i;
    return -1;
}
LONG RegOpenKeyExW(HKEY r, LPCWSTR s, DWORD o, REGSAM a, PHKEY k){ (void)r;(void)s;(void)o;(void)a; *k = (HKEY)1; return ERROR_SUCCESS; }
LONG RegQueryValueExW(HKEY k, LPCWSTR n, LPDWORD t, LPDWORD tt, LPBYTE d, LPDWORD sz){
    (void)k; (void)t; (void)tt;
    int i = fake_find(n);
    if (i < 0) return 2;
    if (d && sz) memcpy(d, &fake[i].val, (*sz < 4) ? *sz : 4);
    if (sz) *sz = 4;
    return ERROR_SUCCESS;
}
LONG RegCreateKeyExW(HKEY r, LPCWSTR s, DWORD o, LPWSTR c, DWORD o2, REGSAM a,
                     LPSECURITY_ATTRIBUTES sa, PHKEY k, LPDWORD d){
    (void)r;(void)s;(void)o;(void)c;(void)o2;(void)a;(void)sa;(void)d; *k = (HKEY)1; return ERROR_SUCCESS;
}
LONG RegSetValueExW(HKEY k, LPCWSTR n, DWORD t, DWORD tt, const BYTE *d, DWORD sz){
    (void)k; (void)t; (void)tt;
    int i = fake_find(n);
    if (i < 0) {
        for (i = 0; i < FAKE_MAX; i++) if (!fake[i].present) { wcsncpy(fake[i].name, n, 31); fake[i].name[31] = 0; break; }
        if (i == FAKE_MAX) return 2;
    }
    if (sz >= 4 && d) memcpy(&fake[i].val, d, 4);
    fake[i].present = 1;
    return ERROR_SUCCESS;
}
LONG RegCloseKey(HKEY k){ (void)k; return ERROR_SUCCESS; }
