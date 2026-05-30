#pragma once
#include <windows.h>

// DPI感知设置
void EnableDpi(void);

// 开机自启
BOOL IsAutoRunEnabled(void);
void SetAutoRun(BOOL enable);
void RefreshAutoRunPath(void);

// DIB位图创建（32位ARGB）
HBITMAP CreateARGBDIB(int w, int h, void **bits);

// 屏幕矩形截图
HBITMAP CaptureScreenRect(int x, int y, int w, int h);

// 将位图复制到剪贴板（CF_BITMAP 格式）
void CopyBitmapToClipboard(HBITMAP bmp);