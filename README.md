# Snipshot v1.2.0

轻量级 Windows 截图 + 屏幕画板工具，基于 Win32 API，MinGW-w64 交叉编译。

## 特性

- **截图贴图** — 区域截图后生成可缩放、可调透明度的悬浮贴图
- **屏幕画板** — 全屏透明画布，画笔/橡皮擦直接在屏幕上标注，抗锯齿平滑线条
- **剪贴板** — 截图自动复制到剪贴板（可选模式）
- **自定义快捷键** — 托盘菜单 → 设置，自由绑定全局热键，注册表持久化
- **系统托盘** — 常驻托盘，右键菜单访问全部功能，支持开机自启
- **多显示器** — Per-Monitor V2 DPI 感知，虚拟屏幕坐标

## 使用说明

### 截图模式

| 操作 | 默认快捷键 |
|---|---|
| 选区截图 → 贴图 | `Ctrl+Shift+S` |
| 选区截图 → 贴图 + 复制 | `Ctrl+Shift+A` |
| 选区截图 → 仅复制 | `Ctrl+Shift+D` |

贴图窗口操作：滚轮缩放 · `Ctrl+滚轮` 调透明度 · 右键关闭。

### 屏幕画板

| 操作 | 默认快捷键 / 方式 |
|---|---|
| 进入画板 | `Ctrl+Shift+B` |
| 画笔绘制 | 左键拖拽 |
| 橡皮擦 | 右键按住 |
| 调节粗细 | 鼠标滚轮 |
| 穿透操作后方窗口 | 按住 `Alt` |
| 退出画板 | `Esc` 或工具栏 × |

工具栏提供：6 色调色板 · 当前颜色和粗细显示 · Clear 清屏 · × 退出。

> 快捷键可在托盘菜单 **设置** 中自定义，修改即时生效。

## 构建

```bash
make
```

输出 `Snipshot.exe`。需要 `x86_64-w64-mingw32-gcc`（MinGW-w64）。

Wine 下可测试运行（部分功能受限）：

```bash
wine Snipshot.exe
```

## 目录结构

```
src/
├── main.c          # 入口、托盘菜单、热键调度
├── region.c        # 区域选取全屏覆盖
├── image_window.c  # 贴图窗口（缩放、透明度、DPI）
├── drawing.c       # 屏幕画板（全屏画布、工具栏、抗锯齿绘制）
├── hotkey.c        # 快捷键注册表读写、格式化
├── settings.c      # 设置界面（快捷键自定义绑定）
├── about.c         # 关于对话框
└── utils.c         # DPI、注册表、位图创建、屏幕截图、剪贴板
res/
├── icon.ico        # 应用图标
├── app.manifest    # 应用程序清单
└── resource.rc     # 资源脚本
Makefile            # MinGW-w64 交叉编译
```

## 作者

**Eetherrr** — [github.com/Etherre/snipshot](https://github.com/Etherre/snipshot)

## 许可证

MIT License，详见 `LICENSE.md`。