# Snipshot

轻量级 Windows 截图 + 屏幕画板工具，基于 Win32 API，MinGW-w64 交叉编译。

## 特性

- **截图贴图** — 区域截图后生成可缩放、可调透明度的悬浮贴图
- **屏幕画板** — 全屏透明画布，画笔/橡皮擦直接在屏幕上标注
- **剪贴板** — 截图自动复制到剪贴板（可选）
- **系统托盘** — 常驻托盘，全局热键触发，开机自启
- **多显示器** — Per-Monitor V2 DPI 感知，虚拟屏幕坐标支持

## 使用说明

### 截图

| 热键 | 行为 |
|---|---|
| `Ctrl+Shift+S` | 选区截图 → 贴图 |
| `Ctrl+Shift+A` | 选区截图 → 贴图 + 复制到剪贴板 |
| `Ctrl+Shift+D` | 选区截图 → 仅复制到剪贴板 |

贴图窗口：滚轮缩放，`Ctrl+滚轮` 调透明度，右键关闭。

### 屏幕画板

| 热键 | 行为 |
|---|---|
| `Ctrl+Shift+B` | 进入画板模式 |

画板操作：

| 操作 | 效果 |
|---|---|
| 左键拖拽 | 画笔绘制 |
| 右键按住 | 橡皮擦 |
| 鼠标滚轮 | 调节粗细 |
| 按住 `Alt` | 临时穿透（操作后方窗口） |
| `Esc` | 退出画板 |

工具栏：色板选择颜色，显示当前颜色和粗细，`Clear` 清屏，`X` 退出。

## 构建

```bash
make
```

输出 `Snipshot.exe`。需要 `x86_64-w64-mingw32-gcc`。

Wine 测试：

```bash
wine Snipshot.exe
```

## 目录结构

- `src/main.c` — 程序入口、托盘菜单、热键调度
- `src/region.c` — 区域选取全屏覆盖
- `src/image_window.c` — 贴图窗口（缩放、透明度、DPI 适配）
- `src/drawing.c` — 屏幕画板（全屏画布、工具栏、抗锯齿绘制）
- `src/about.c` — 关于对话框
- `src/utils.c` — DPI、注册表、位图、剪贴板
- `res/` — 图标与应用清单

## 作者

**Eetherrr** — [github.com/Etherre/snipshot](https://github.com/Etherre/snipshot)

## 许可证

MIT License，详见 `LICENSE.md`。