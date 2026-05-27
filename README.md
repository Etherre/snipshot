# Snipshot

Snipshot 是一个轻量级的 Windows 截图工具，基于 Win32 API 开发，支持区域截图后直接在屏幕上显示为可缩放、可透明的贴图窗口。

## 特性

- 系统托盘图标常驻
- 全局热键 `Ctrl+Shift+S` 触发区域截图
- 区域选取后自动生成贴图窗口
- 鼠标滚轮缩放截图贴图
- `Ctrl + 滚轮` 调节贴图透明度
- 右键关闭贴图窗口
- 托盘菜单支持「开机自启」和「退出」
- 多显示器支持，使用虚拟屏幕坐标进行捕获
- 单实例运行，避免重复启动

## 目录结构

- `src/main.c` - 程序入口、托盘菜单、热键与截图逻辑
- `src/region.c` - 区域选取窗口实现
- `src/image_window.c` - 贴图窗口绘制与交互处理
- `src/utils.c` - DPI 设置、开机自启、屏幕截图、DIB 创建
- `src/*.h` - 头文件接口定义
- `res/resource.rc` - 程序图标与清单资源
- `res/app.manifest` - 应用程序清单
- `Makefile` - 使用 MinGW-w64 交叉编译生成 `Snipshot.exe`

## 构建

该项目使用 `x86_64-w64-mingw32-gcc` 交叉编译器构建。如果系统中已安装 MinGW-w64，则直接运行：

```bash
make
```

输出文件：

```bash
Snipshot.exe
```

如果需要指定不同编译器或路径，可直接覆写 `CC` 变量：

```bash
make CC=x86_64-w64-mingw32-gcc
```

## 运行

生成的 `Snipshot.exe` 是 Windows 可执行文件，可在 Windows 环境下直接运行。

如果在 Linux 上测试，可使用 Wine：

```bash
wine Snipshot.exe
```

## 使用说明

1. 启动程序后，程序在系统托盘中常驻。
2. 按 `Ctrl+Shift+S` 或从托盘菜单选择「选区截图并贴图」。
3. 在屏幕上拖动以选取截图区域。
4. 选区截图完成后，会弹出一个贴图窗口。
5. 在贴图窗口上使用滚轮：
   - 普通滚轮：缩放图像
   - `Ctrl + 滚轮`：调整透明度
6. 右键点击贴图窗口可关闭该窗口。
7. 通过托盘菜单切换「开机自启」。

## 依赖

- MinGW-w64 交叉编译工具链
- Windows API（运行时）

## 备注

- `src/utils.c` 中的 `RefreshAutoRunPath()` 会在程序启动时检查并刷新注册表中的开机自启路径，避免路径失效。
- 贴图窗口采用 `WS_EX_LAYERED` 的分层窗口，实现透明度和无边框显示。
- 区域选取窗口使用虚拟屏幕坐标，使其支持多个显示器。

## 许可证

本项目基于 MIT 许可证发布。详情请参见 `LICENSE` 文件。
