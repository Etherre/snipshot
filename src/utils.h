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
HBITMAP CaptureScreenRect(int x, int y, int w, int h, void **bits);