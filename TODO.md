待办清单（建议优先级）
P0 — 功能/稳定性修复

- [x] 修橡皮擦：DrawDot 核心分支对 alpha != 255（橡皮）无条件置 px[3] = 1
- [x] 防重入：RunDrawingMode/RunRegionSelection/两个对话框入口加 guard（如 if (IsWindow(g_draw.hwndCanvas)) { SetForegroundWindow(...); return; }）

P1 — UI 正确性

- [ ] about/settings 统一改用 GetClientRect 计算按钮矩形（或把命中/绘制都存进全局 RECT），修死按钮与裁切
- [ ] 修复 GDI 泄漏：删除前先 SelectObject 还原旧对象，或缓存笔/字体复用
- [ ] About 改用 LoadHotkeys 实际值显示快捷键

P2 — 体验与健壮性

- [ ] RegisterHotKey 失败检测（冲突提示）；OpenClipboard 重试
- [ ] 高 DPI 自绘 UI 缩放；混合 DPI 坐标审计
- [ ] WM_TASKBARCREATED 恢复托盘；第二实例唤起首实例；选区/画板入口 SetForegroundWindow

P3 — 工程化

- [ ] Makefile 加 -MMD -MP；重新生成 compile_commands.json；补版本资源；README 修正（LICENSE 引用、Win10 要求）；.gitignore 加 tests/run_tests*
- [ ] 把 make -C tests test 纳入 CI/提交前检查（橡皮擦/防重入回归断言已就位）