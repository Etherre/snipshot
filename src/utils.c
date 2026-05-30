#include "utils.h"
#include <shellapi.h>

void EnableDpi(void)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
}

BOOL IsAutoRunEnabled(void)
{
    HKEY hKey;
    BOOL result = FALSE;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
                      L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD type, size;
        if (RegQueryValueExW(hKey, L"Snipshot", NULL, &type, NULL, &size) == ERROR_SUCCESS
            && type == REG_SZ)
        {
            result = TRUE;
        }
        RegCloseKey(hKey);
    }
    return result;
}

void SetAutoRun(BOOL enable)
{
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
                      L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                      0, KEY_SET_VALUE | KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        if (enable) {
            wchar_t exePath[MAX_PATH];
            GetModuleFileNameW(NULL, exePath, MAX_PATH);
            RegSetValueExW(hKey, L"Snipshot", 0, REG_SZ,
                          (BYTE*)exePath,
                          (wcslen(exePath) + 1) * sizeof(wchar_t));
        } else {
            RegDeleteValueW(hKey, L"Snipshot");
        }
        RegCloseKey(hKey);
    }
}

void RefreshAutoRunPath(void)
{
    if (IsAutoRunEnabled()) {
        SetAutoRun(TRUE);
    }
}

HBITMAP CreateARGBDIB(int w, int h, void **bits)
{
    BITMAPINFO bi = {0};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    HDC hdc = GetDC(NULL);
    HBITMAP bmp = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, bits, NULL, 0);
    ReleaseDC(NULL, hdc);
    return bmp;
}

HBITMAP CaptureScreenRect(int x, int y, int w, int h)
{
    void *bits;
    HBITMAP bmp = CreateARGBDIB(w, h, &bits);
    if (!bmp) return NULL;

    HDC screen = GetDC(NULL);
    HDC mem = CreateCompatibleDC(screen);
    HBITMAP old = SelectObject(mem, bmp);
    BitBlt(mem, 0, 0, w, h, screen, x, y, SRCCOPY | CAPTUREBLT);
    SelectObject(mem, old);
    DeleteDC(mem);
    ReleaseDC(NULL, screen);
    return bmp;
}

void CopyBitmapToClipboard(HBITMAP bmp)
{
    if (!OpenClipboard(NULL)) return;

    EmptyClipboard();

    HBITMAP hCopy = (HBITMAP)CopyImage(bmp, IMAGE_BITMAP, 0, 0, 0);
    if (hCopy)
        SetClipboardData(CF_BITMAP, hCopy);

    CloseClipboard();
}