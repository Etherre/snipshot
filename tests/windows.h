/* Minimal Windows shim: types/macros/constants used by drawing.c & hotkey.c.
 * Linux-host unit tests only — never linked into the Windows binary. */
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

typedef int                 BOOL;
typedef int                 INT;
#define TRUE  1
#define FALSE 0
typedef unsigned char       BYTE;
typedef unsigned int        UINT;
typedef unsigned int         DWORD;   /* LLP64: 32-bit */
typedef unsigned short       WORD;
typedef int                  LONG;    /* LLP64: 32-bit */
typedef short                SHORT;
typedef unsigned int         ULONG;
typedef long long            LONGLONG;
typedef intptr_t            LONG_PTR;
typedef uintptr_t           ULONG_PTR;
typedef uintptr_t           DWORD_PTR;
typedef uintptr_t           UINT_PTR;
typedef intptr_t            LPARAM;
typedef uintptr_t           WPARAM;
typedef LONG_PTR            LRESULT;
typedef const wchar_t      *LPCWSTR;
typedef wchar_t            *LPWSTR;
typedef const char         *LPCSTR;
typedef BYTE               *LPBYTE;
typedef DWORD              *LPDWORD;
typedef void               *LPVOID;
typedef void               *HANDLE;
typedef void               *HGDIOBJ;
typedef void               *HINSTANCE;
typedef void               *HWND;
typedef void               *HDC;
typedef void               *HBITMAP;
typedef void               *HBRUSH;
typedef void               *HPEN;
typedef void               *HFONT;
typedef void               *HICON;
typedef void               *HCURSOR;
typedef void               *HMENU;
typedef void               *HKEY;
typedef HKEY              *PHKEY;
typedef DWORD               REGSAM;
typedef void               *LPSECURITY_ATTRIBUTES;
typedef WORD                ATOM;
typedef void               *LPCVOID;

typedef struct { LONG x, y; } POINT;
typedef struct { LONG left, top, right, bottom; } RECT;
typedef struct { LONG cx, cy; } SIZE;
typedef struct { BYTE BlendOp, BlendFlags, SourceConstantAlpha, AlphaFormat; } BLENDFUNCTION;
typedef struct { HWND hwnd; UINT message; WPARAM wParam; LPARAM lParam; DWORD time; POINT pt; DWORD lPrivate; } MSG;
typedef struct { HDC hdc; BOOL fErase; RECT rcPaint; BOOL fRestore; BOOL fIncUpdate; BYTE rgbReserved[32]; } PAINTSTRUCT;
typedef LRESULT (*WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef void (*TIMERPROC)(HWND, UINT, UINT_PTR, DWORD);
typedef struct {
    UINT style; WNDPROC lpfnWndProc; int cbClsExtra, cbWndExtra;
    HINSTANCE hInstance; HICON hIcon; HCURSOR hCursor; HBRUSH hbrBackground;
    LPCWSTR lpszMenuName; LPCWSTR lpszClassName;
} WNDCLASSW;
typedef struct { HDC hdc; LONG x, y; } nothing_t; /* unused */

#define CALLBACK
#define WINAPI
#define WINUSERAPI
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define LOWORD(l)  ((WORD)((DWORD_PTR)(l) & 0xffff))
#define HIWORD(l)  ((WORD)(((DWORD_PTR)(l) >> 16) & 0xffff))
#define RGB(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
typedef DWORD COLORREF;
#define GetRValue(c) ((BYTE)(c))
#define GetGValue(c) ((BYTE)((WORD)(c) >> 8))
#define GetBValue(c) ((BYTE)((c) >> 16))

/* messages */
#define WM_CREATE        0x0001
#define WM_DESTROY       0x0002
#define WM_PAINT         0x000F
#define WM_KEYDOWN       0x0100
#define WM_KEYUP         0x0101
#define WM_SYSKEYDOWN    0x0104
#define WM_SYSKEYUP      0x0105
#define WM_TIMER         0x0113
#define WM_MOUSEMOVE     0x0200
#define WM_LBUTTONDOWN   0x0201
#define WM_LBUTTONUP     0x0202
#define WM_RBUTTONDOWN   0x0204
#define WM_RBUTTONUP     0x0205
#define WM_MBUTTONDOWN   0x0207
#define WM_MBUTTONUP     0x0208
#define WM_MOUSEWHEEL    0x020A
#define WM_NCLBUTTONDOWN 0x00A1
#define WM_QUIT          0x0012

/* styles */
#define WS_POPUP          0x80000000u
#define WS_VISIBLE        0x10000000u
#define WS_CAPTION        0x00C00000u
#define WS_SYSMENU        0x00080000u
#define WS_EX_LAYERED     0x00080000u
#define WS_EX_TOPMOST     0x00000008u
#define WS_EX_TOOLWINDOW  0x00000080u
#define WS_EX_NOACTIVATE  0x08000000u
#define WS_EX_TRANSPARENT 0x00000020u
#define GWL_EXSTYLE      (-20)
#define GWLP_USERDATA    (-21)
#define SWP_NOSIZE        0x0001
#define SWP_NOMOVE        0x0002
#define SWP_NOZORDER      0x0004
#define SWP_NOACTIVATE    0x0010
#define SWP_FRAMECHANGED  0x0020
#define SW_SHOW           5
#define HTCAPTION         2

/* GDI */
#define PS_SOLID          0
#define TRANSPARENT       1
#define DEFAULT_CHARSET   1
#define OUT_DEFAULT_PRECIS 0
#define CLIP_DEFAULT_PRECIS 0
#define CLEARTYPE_QUALITY 5
#define DEFAULT_PITCH     0
#define FW_NORMAL         400
#define FW_SEMIBOLD       600
#define FW_BOLD           700
#define DT_LEFT           0x00000000u
#define DT_CENTER         0x00000001u
#define DT_RIGHT          0x00000002u
#define DT_VCENTER        0x00000004u
#define DT_SINGLELINE     0x00000020u
#define DT_CALCRECT       0x00000400u
#define SRCCOPY           0x00CC0020u
#define AC_SRC_OVER       0x00
#define AC_SRC_ALPHA      0x01
#define ULW_ALPHA         0x00000002u
#define LWA_ALPHA         0x00000002u
#define NULL_BRUSH        5
#define HOLLOW_BRUSH      5
#define BLACK_BRUSH       4
#define IDC_ARROW         ((LPCWSTR)32512)
#define IDC_CROSS         ((LPCWSTR)32515)

/* virtual keys */
#define VK_SHIFT          0x10
#define VK_CONTROL        0x11
#define VK_MENU           0x12
#define VK_LMENU          0xA4
#define VK_RMENU          0xA5
#define VK_ESCAPE         0x1B
#define VK_LEFT           0x25
#define VK_UP             0x26
#define VK_RIGHT          0x27
#define VK_DOWN           0x28
#define VK_PRIOR          0x21
#define VK_NEXT           0x22
#define VK_END            0x23
#define VK_HOME           0x24
#define VK_INSERT         0x2D
#define VK_DELETE         0x2E
#define VK_DIVIDE         0x6F
#define VK_F1             0x70
#define VK_F2             0x71
#define VK_F3             0x72
#define VK_F4             0x73
#define VK_F5             0x74
#define VK_F6             0x75
#define VK_F7             0x76
#define VK_F8             0x77
#define VK_F9             0x78
#define VK_F10            0x79
#define VK_F11            0x7A
#define VK_F12            0x7B
#define VK_F13            0x7C
#define VK_F14            0x7D
#define VK_F15            0x7E
#define VK_F16            0x7F
#define VK_F17            0x80
#define VK_F18            0x81
#define VK_F19            0x82
#define VK_F20            0x83
#define VK_F21            0x84
#define VK_F22            0x85
#define VK_F23            0x86
#define VK_F24            0x87
#define VK_OEM_PLUS       0xBB
#define VK_OEM_COMMA      0xBC
#define VK_OEM_MINUS      0xBD
#define VK_OEM_PERIOD     0xBE

/* hotkeys / registry */
#define MOD_ALT           0x0001
#define MOD_CONTROL       0x0002
#define MOD_SHIFT         0x0004
#define MAPVK_VK_TO_VSC   0
#define ERROR_SUCCESS     0
#define KEY_READ          0x20019
#define KEY_SET_VALUE     0x0002
#define REG_OPTION_NON_VOLATILE 0
#define REG_DWORD         4
#define HKEY_CURRENT_USER ((HKEY)(intptr_t)0x80000001)

/* system metrics */
#define SM_CXSCREEN       0
#define SM_CYSCREEN       1
#define SM_XVIRTUALSCREEN 76
#define SM_YVIRTUALSCREEN 77
#define SM_CXVIRTUALSCREEN 78
#define SM_CYVIRTUALSCREEN 79

/* ── function declarations (stubbed in win_stubs.c) ── */
HDC       GetDC(HWND);
int       ReleaseDC(HWND, HDC);
HDC       CreateCompatibleDC(HDC);
BOOL      DeleteDC(HDC);
HBITMAP   CreateCompatibleBitmap(HDC, int, int);
HBITMAP   CreateARGBDIB(int, int, void **);
BOOL      UpdateLayeredWindow(HWND, HDC, POINT *, SIZE *, HDC, POINT *, COLORREF, BLENDFUNCTION *, DWORD);
LONG_PTR  GetWindowLongPtrW(HWND, int);
LONG_PTR  SetWindowLongPtrW(HWND, int, LONG_PTR);
BOOL      SetWindowPos(HWND, HWND, int, int, int, int, UINT);
int       FillRect(HDC, const RECT *, HBRUSH);
HBRUSH    CreateSolidBrush(COLORREF);
BOOL      DeleteObject(HGDIOBJ);
HPEN      CreatePen(int, int, COLORREF);
HGDIOBJ   SelectObject(HDC, HGDIOBJ);
BOOL      Rectangle(HDC, int, int, int, int);
HDC       BeginPaint(HWND, PAINTSTRUCT *);
BOOL      GetClientRect(HWND, RECT *);
BOOL      EndPaint(HWND, const PAINTSTRUCT *);
int       DrawTextW(HDC, LPCWSTR, int, RECT *, UINT);
int       SetBkMode(HDC, int);
COLORREF  SetTextColor(HDC, COLORREF);
BOOL      Ellipse(HDC, int, int, int, int);
int       wsprintfW(LPWSTR, LPCWSTR, ...);
BOOL      InvalidateRect(HWND, const RECT *, BOOL);
BOOL      DestroyWindow(HWND);
BOOL      PostMessageW(HWND, UINT, WPARAM, LPARAM);
DWORD     GetTickCount(void);
HWND      SetCapture(HWND);
BOOL      ReleaseCapture(void);
SHORT     GetAsyncKeyState(int);
UINT_PTR  SetTimer(HWND, UINT_PTR, UINT, TIMERPROC);
BOOL      KillTimer(HWND, UINT_PTR);
HWND      SetFocus(HWND);
HWND      GetFocus(void);
BOOL      ShowWindow(HWND, int);
ATOM      RegisterClassW(const WNDCLASSW *);
HCURSOR   LoadCursor(HINSTANCE, LPCWSTR);
int       GetSystemMetrics(int);
HWND      CreateWindowExW(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
BOOL      GetMessageW(MSG *, HWND, UINT, UINT);
BOOL      TranslateMessage(const MSG *);
LRESULT   DispatchMessageW(const MSG *);
BOOL      IsWindow(HWND);
LRESULT   DefWindowProcW(HWND, UINT, WPARAM, LPARAM);
HGDIOBJ   GetStockObject(int);
BOOL      PtInRect(const RECT *, POINT);
BOOL      SetLayeredWindowAttributes(HWND, COLORREF, BYTE, DWORD);
BOOL      BitBlt(HDC, int, int, int, int, HDC, int, int, DWORD);
HFONT     CreateFontW(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD,
                      DWORD, DWORD, DWORD, DWORD, LPCWSTR);
BOOL      MoveToEx(HDC, int, int, POINT *);
BOOL      LineTo(HDC, int, int);
HINSTANCE ShellExecuteW(HWND, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, INT);
BOOL      SetForegroundWindow(HWND);
UINT      MapVirtualKeyW(UINT, UINT);
int       GetKeyNameTextW(LONG, LPWSTR, int);
LONG      RegOpenKeyExW(HKEY, LPCWSTR, DWORD, REGSAM, PHKEY);
LONG      RegQueryValueExW(HKEY, LPCWSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
LONG      RegCreateKeyExW(HKEY, LPCWSTR, DWORD, LPWSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD);
LONG      RegSetValueExW(HKEY, LPCWSTR, DWORD, DWORD, const BYTE *, DWORD);
LONG      RegCloseKey(HKEY);

/* test harness helpers */
void      fake_registry_clear(void);
int       fake_registry_count(void);
void      fake_set_keynames(int on);
void      fake_clear_foreground(void);
HWND      fake_last_foreground(void);
void      fake_clear_invalidate(void);
BOOL      fake_last_invalidate(RECT *out);
