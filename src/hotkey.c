#include "hotkey.h"

#define REG_KEY L"Software\\Snipshot\\Hotkeys"

static const HotkeyAction kActions[HK_COUNT] = {
    { L"Pin",     L"Screenshot + Pin",    MOD_CONTROL | MOD_SHIFT, 'S' },
    { L"PinCopy", L"Pin + Copy",          MOD_CONTROL | MOD_SHIFT, 'A' },
    { L"Copy",    L"Screenshot + Copy",   MOD_CONTROL | MOD_SHIFT, 'D' },
    { L"Drawing", L"Drawing Board",       MOD_CONTROL | MOD_SHIFT, 'B' },
};

const HotkeyAction *GetHotkeyActions(void)
{
    return kActions;
}

static DWORD PackHotkey(UINT mods, UINT vk)
{
    return ((DWORD)mods << 16) | (DWORD)vk;
}

static void UnpackHotkey(DWORD packed, UINT *mods, UINT *vk)
{
    *mods = (UINT)(packed >> 16);
    *vk   = (UINT)(packed & 0xFFFF);
}

void LoadHotkeys(UINT outMods[HK_COUNT], UINT outVks[HK_COUNT])
{
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, REG_KEY,
                      0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        for (int i = 0; i < HK_COUNT; i++) {
            DWORD packed = 0, size = sizeof(DWORD);
            if (RegQueryValueExW(hKey, kActions[i].name,
                                 NULL, NULL, (BYTE*)&packed, &size) == ERROR_SUCCESS
                && size == sizeof(DWORD) && packed != 0)
            {
                UnpackHotkey(packed, &outMods[i], &outVks[i]);
            } else {
                outMods[i] = kActions[i].defMod;
                outVks[i]  = kActions[i].defVk;
            }
        }
        RegCloseKey(hKey);
    }
    else
    {
        // 注册表项不存在，全部使用默认值
        for (int i = 0; i < HK_COUNT; i++) {
            outMods[i] = kActions[i].defMod;
            outVks[i]  = kActions[i].defVk;
        }
    }
}

void SaveHotkey(const wchar_t *name, UINT mods, UINT vk)
{
    HKEY hKey;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, REG_KEY,
                        0, NULL, REG_OPTION_NON_VOLATILE,
                        KEY_SET_VALUE, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        DWORD packed = PackHotkey(mods, vk);
        RegSetValueExW(hKey, name, 0, REG_DWORD,
                       (BYTE*)&packed, sizeof(DWORD));
        RegCloseKey(hKey);
    }
}

void ResetHotkeys(void)
{
    HKEY hKey;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, REG_KEY,
                        0, NULL, REG_OPTION_NON_VOLATILE,
                        KEY_SET_VALUE, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        for (int i = 0; i < HK_COUNT; i++) {
            DWORD packed = PackHotkey(kActions[i].defMod, kActions[i].defVk);
            RegSetValueExW(hKey, kActions[i].name, 0, REG_DWORD,
                           (BYTE*)&packed, sizeof(DWORD));
        }
        RegCloseKey(hKey);
    }
}

void FormatHotkey(UINT mods, UINT vk, wchar_t *buf, int len)
{
    wchar_t part[64] = {0};
    if (mods & MOD_CONTROL) wcscat(part, L"Ctrl+");
    if (mods & MOD_SHIFT)   wcscat(part, L"Shift+");
    if (mods & MOD_ALT)     wcscat(part, L"Alt+");

    // GetKeyNameText 可处理大多数按键
    UINT sc = MapVirtualKeyW(vk, MAPVK_VK_TO_VSC);
    switch (vk) {
        case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN:
        case VK_PRIOR: case VK_NEXT: case VK_END: case VK_HOME:
        case VK_INSERT: case VK_DELETE:
        case VK_DIVIDE:
            sc |= 0x100; // 扩展键标志
            break;
    }
    wchar_t keyName[32] = L"";
    GetKeyNameTextW(sc << 16, keyName, 32);

    if (keyName[0]) {
        wcscat(part, keyName);
    } else if (vk >= 'A' && vk <= 'Z') {
        int l = (int)wcslen(part);
        part[l] = (wchar_t)vk;
        part[l+1] = L'\0';
    } else if (vk >= '0' && vk <= '9') {
        int l = (int)wcslen(part);
        part[l] = (wchar_t)vk;
        part[l+1] = L'\0';
    } else if (vk >= VK_F1 && vk <= VK_F24) {
        wsprintfW(part + wcslen(part), L"F%d", vk - VK_F1 + 1);
    } else if (vk == VK_OEM_MINUS) {
        wcscat(part, L"-");
    } else if (vk == VK_OEM_PLUS) {
        wcscat(part, L"=");
    } else if (vk == VK_OEM_COMMA) {
        wcscat(part, L",");
    } else if (vk == VK_OEM_PERIOD) {
        wcscat(part, L".");
    } else {
        wsprintfW(part + wcslen(part), L"VK_%u", vk);
    }

    wcsncpy(buf, part, (size_t)len);
    buf[len - 1] = L'\0';
}