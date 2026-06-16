#pragma once
#include <windows.h>

// 注册画板相关窗口类
BOOL RegisterDrawingClass(HINSTANCE hInst);

// 进入画板模式（阻塞，直到用户退出）
void RunDrawingMode(HINSTANCE hInst);