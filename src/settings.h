#pragma once
#include <windows.h>

// 注册设置窗口类
BOOL RegisterSettingsClass(HINSTANCE hInst);

// 显示设置对话框，返回 TRUE 表示快捷键有改动
BOOL ShowSettingsDialog(HINSTANCE hInst);