# Snipshot

Snipshot 是一个轻量级的 Windows 截图工具，基于 Win32 API 开发，支持区域截图后贴图显示或复制到剪贴板。

## 特性

- 系统托盘图标常驻，单实例运行
- 三种截图模式，各有独立热键和托盘菜单入口
- 贴图窗口支持鼠标滚轮缩放、`Ctrl + 滚轮`调节透明度
- 贴图边缘 1px 边框，浅色截图在浅色背景上也清晰可见
- 滚轮缩放 16ms 节流，避免快速滚动时瞬时大量内存分配
- 截图自动复制到剪贴板（可选）
- 托盘菜单支持「开机自启」和「退出」
- 多显示器支持，Per-Monitor V2 DPI 感知

## 使用说明

| 热键 | 行为 |
|---|---|
| `Ctrl+Shift+S` | 选区截图 → 贴图 |
| `Ctrl+Shift+A` | 选区截图 → 贴图 + 复制到剪贴板 |
| `Ctrl+Shift+D` | 选区截图 → 仅复制到剪贴板 |

选区操作：鼠标拖拽选取区域，`Esc` 取消。贴图窗口：右键关闭。

## 构建

使用 `x86_64-w64-mingw32-gcc` 交叉编译：

```bash
make
```

输出 `Snipshot.exe`，在 Windows 下直接运行。也可指定编译器：

```bash
make CC=x86_64-w64-mingw32-gcc
```

Wine 下可测试运行：

```bash
wine Snipshot.exe
```

## 目录结构

- `src/main.c` — 程序入口、托盘菜单、热键与截图模式调度
- `src/region.c` — 区域选取全屏覆盖窗口
- `src/image_window.c` — 贴图窗口（分层窗口、缩放、透明度、DPI 适配）
- `src/utils.c` — DPI 设置、开机自启注册表、位图创建、屏幕截图、剪贴板
- `src/*.h` — 头文件接口
- `res/` — 图标与应用程序清单
- `Makefile` — MinGW-w64 交叉编译

## 依赖

- MinGW-w64 交叉编译工具链
- Windows API：`gdi32` `user32` `shell32` `advapi32`

## 许可证

MIT License，详见 `LICENSE.md`。