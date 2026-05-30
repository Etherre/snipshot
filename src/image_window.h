#pragma once
#include <windows.h>

typedef struct {
    HBITMAP bmp;          // 当前显示位图（缩放后）
    int     width;
    int     height;
    int     alpha;        // 透明度 0-255

    /* 缩放相关 */
    HBITMAP orig_bmp;     // 原始截图（始终不变）
    int     orig_width;
    int     orig_height;
    float   scale;        // 当前缩放倍数

    DWORD   last_wheel_time; // 滚轮节流
} IMAGE_WINDOW;

// 注册贴图窗口类
BOOL RegisterImageClass(HINSTANCE hInst);

// 创建并显示贴图窗口
HWND CreateImageWindow(HINSTANCE hInst, int x, int y, int w, int h, HBITMAP origBmp);