#pragma once
#include <windows.h>

// 注册区域选择窗口类
BOOL RegisterRegionClass(HINSTANCE hInst);

// 运行区域选择，返回有效选区
BOOL RunRegionSelection(HINSTANCE hInst, RECT *outRect);