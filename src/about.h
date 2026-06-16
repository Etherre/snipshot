#pragma once
#include <windows.h>

// 注册关于窗口类
BOOL RegisterAboutClass(HINSTANCE hInst);

// 显示关于对话框
void ShowAboutDialog(HINSTANCE hInst);