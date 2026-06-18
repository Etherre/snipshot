#pragma once
#include <windows.h>

#define HK_COUNT 4

/* 单个快捷键定义 */
typedef struct {
    const wchar_t *name;     // 注册表值名
    const wchar_t *label;    // 显示标签
    UINT           defMod;   // 默认修饰键
    UINT           defVk;    // 默认虚拟键码
} HotkeyAction;

/* 获取 4 个操作的静态数组 */
const HotkeyAction *GetHotkeyActions(void);

/* 从注册表加载快捷键（缺省返回默认值） */
void LoadHotkeys(UINT outMods[HK_COUNT], UINT outVks[HK_COUNT]);

/* 保存单个快捷键到注册表 */
void SaveHotkey(const wchar_t *name, UINT mods, UINT vk);

/* 恢复所有快捷键到默认值 */
void ResetHotkeys(void);

/* 格式化快捷键为显示字符串（如 "Ctrl+Shift+S"） */
void FormatHotkey(UINT mods, UINT vk, wchar_t *buf, int len);