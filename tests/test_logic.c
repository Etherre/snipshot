/* Unit tests for the pure logic in src/drawing.c, hotkey.c, region.c,
 * settings.c, about.c — compiled with the REAL source files on Linux via the
 * tests/windows.h shim.
 *
 *   make -C tests test
 */
#define _POSIX_C_SOURCE 199309L  /* clock_gettime */
#include <stdio.h>
#include <time.h>
#include "windows.h"

/* Pull in the actual production sources so we test the real code. */
#include "../src/drawing.c"
#include "../src/hotkey.c"
#include "../src/region.c"
#include "../src/settings.c"
#include "../src/about.c"

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, ...) do { \
    if (cond) { g_pass++; } \
    else { g_fail++; printf("FAIL %s:%d: ", __FILE__, __LINE__); \
           printf(__VA_ARGS__); printf("\n"); } \
} while (0)

static BYTE *px(int x, int y)
{
    return g_draw.canvas_bits + ((size_t)y * g_draw.w + (size_t)x) * 4;
}

static void setup_canvas(int w, int h)
{
    free(g_draw.canvas_bits); /* 释放上一次画布，保证 ASan 无泄漏 */
    memset(&g_draw, 0, sizeof(g_draw));
    g_draw.w = w;
    g_draw.h = h;
    g_draw.canvas_bits = (BYTE *)malloc((size_t)w * h * 4);
    memset(g_draw.canvas_bits, 0x01, (size_t)w * h * 4); /* alpha=1 全透明 */
}

int main(void)
{
    printf("=== drawing.c: 像素绘制 ===\n");

    /* 1. ClearCanvas */
    setup_canvas(80, 60);
    memset(g_draw.canvas_bits, 0xAB, (size_t)80 * 60 * 4);
    ClearCanvas();
    {
        DWORD *q = (DWORD *)g_draw.canvas_bits;
        int ok = 1;
        for (int i = 0; i < 80 * 60; i++) if (q[i] != 0x01000000u) ok = 0;
        CHECK(ok, "ClearCanvas 未把全部像素置为 BGRA(0,0,0,1)");
    }

    /* 2. DrawDot 核心不透明 + 颜色正确 (BGRA 顺序) */
    setup_canvas(100, 100);
    DrawDot(50, 50, 5, RGB(255, 0, 0), 255);
    CHECK(px(50,50)[0] == 0 && px(50,50)[1] == 0 &&
          px(50,50)[2] == 255 && px(50,50)[3] == 255,
          "DrawDot 中心应为不透明白=0 绿=0 红=255 a=255，实际 b=%u g=%u r=%u a=%u",
          px(50,50)[0], px(50,50)[1], px(50,50)[2], px(50,50)[3]);

    /* 3. 抗锯齿过渡带：d2=25 处 alpha 应恰为 139 */
    CHECK(px(55,50)[3] == 139, "AA 过渡带 d2=25 期望 alpha=139，实际 %u", px(55,50)[3]);
    CHECK(px(57,50)[3] == 1, "远离圆心的像素不应被修改，实际 alpha=%u", px(57,50)[3]);

    /* 4. 越界中心不崩溃且不影响画布 */
    DrawDot(-20, -20, 5, RGB(0,255,0), 255);
    DrawDot(120, 120, 5, RGB(0,255,0), 255);
    CHECK(px(0,0)[3] == 1 && px(99,99)[3] == 1, "越界 DrawDot 不应写入画布");

    /* 5. 真实场景：画笔一笔之后，用橡皮擦在同位置擦除（回归：曾擦不掉实心笔迹） */
    setup_canvas(100, 100);
    DrawDot(50, 50, 5, RGB(255, 0, 0), 255);   /* 画笔 */
    CHECK(px(50,50)[3] == 255, "画笔核心应不透明");
    DrawDot(50, 50, 5, RGB(0,0,0), 1);         /* 橡皮擦同位置 */
    CHECK(px(50,50)[3] == 1, "橡皮擦应擦除实心笔迹核心（alpha=%u，期望 1）", px(50,50)[3]);
    CHECK(px(52,53)[3] == 1, "橡皮擦应擦除笔迹内侧 d2=13（alpha=%u）", px(52,53)[3]);
    CHECK(px(55,50)[3] == 1, "橡皮擦应擦除过渡带 d2=25（alpha=%u）", px(55,50)[3]);
    CHECK(px(55,52)[3] == 1, "橡皮擦应擦除过渡带 d2=29（alpha=%u）", px(55,52)[3]);
    CHECK(px(57,50)[3] == 1, "橡皮擦足迹外画布应保持透明（alpha=%u）", px(57,50)[3]);
    /* 擦除后重画应能重新覆盖 */
    DrawDot(50, 50, 3, RGB(0, 0, 255), 255);
    CHECK(px(50,50)[3] == 255 && px(50,50)[2] == 0 && px(50,50)[0] == 255,
          "擦除后重画应完全覆盖（b=%u g=%u r=%u a=%u）",
          px(50,50)[0], px(50,50)[1], px(50,50)[2], px(50,50)[3]);

    /* 6. 硬边界：全不透明画布上，橡皮擦足迹内全擦净、足迹外不动 */
    {
        for (size_t i = 0; i < (size_t)100 * 100 * 4; i++) g_draw.canvas_bits[i] = 0xFF;
        DrawDot(50, 50, 5, RGB(0,0,0), 1);
        int missed = 0, wrong = 0;
        for (int dy = -9; dy <= 9; dy++)
            for (int dx = -9; dx <= 9; dx++) {
                int d2 = dx*dx + dy*dy;
                BYTE a = px(50+dx, 50+dy)[3];
                if (d2 <= 30) { if (a != 1) missed++; }   /* 笔迹足迹内 */
                else          { if (a != 255) wrong++; }  /* 足迹外 */
            }
        CHECK(missed == 0, "橡皮擦足迹内仍有 %d 个像素未擦净", missed);
        CHECK(wrong == 0, "橡皮擦误擦了 %d 个足迹外像素", wrong);
    }

    /* 6b. 最小半径 r=1 的橡皮擦 */
    {
        setup_canvas(60, 60);
        DrawDot(30, 30, 3, RGB(255,255,255), 255);
        DrawDot(30, 30, 1, RGB(0,0,0), 1);
        CHECK(px(30,30)[3] == 1, "r=1 橡皮擦应擦除中心（alpha=%u）", px(30,30)[3]);
        CHECK(px(31,30)[3] == 1, "r=1 橡皮擦应擦除邻点（alpha=%u）", px(31,30)[3]);
        CHECK(px(32,30)[3] == 255, "r=1 橡皮擦足迹外应保留笔迹（alpha=%u）", px(32,30)[3]);
    }

    /* 7. DrawLine 连通性 */
    setup_canvas(60, 60);
    DrawLine(5, 5, 40, 22, 3, RGB(0,0,255), 255);
    CHECK(px(5,5)[3] == 255, "线段起点不透明");
    CHECK(px(40,22)[3] == 255, "线段终点不透明");
    CHECK(px(22,13)[3] > 1, "线段中点 (22,13) 应被覆盖");
    {
        int covered = 0;
        for (int y = 0; y < 60; y++)
            for (int x = 0; x < 60; x++)
                if (px(x,y)[3] > 1) covered++;
        CHECK(covered >= 36, "线段覆盖像素过少（%d）", covered);
    }
    CHECK(px(55,50)[3] == 1, "远离线段的像素不应被修改");

    /* 7b. 大步距画线（r=8 → 墨点间隔 4）：路径上每个像素都必须完整覆盖 */
    setup_canvas(80, 80);
    DrawLine(10, 10, 60, 40, 8, RGB(255,255,0), 255);
    {
        int x = 10, y = 10;
        int ldx = abs(60 - 10), sx = 1;
        int ldy = -abs(40 - 10), sy = 1;
        int lerr = ldx + ldy;
        int gaps = 0;
        for (;;) {
            if (px(x,y)[3] != 255) gaps++;
            if (x == 60 && y == 40) break;
            int e2 = 2 * lerr;
            if (e2 >= ldy) { lerr += ldy; x += sx; }
            if (e2 <= ldx) { lerr += ldx; y += sy; }
        }
        CHECK(gaps == 0, "大步距画线路径上有 %d 个未覆盖像素", gaps);
        CHECK(px(10,10)[3] == 255 && px(60,40)[3] == 255, "大步距画线端点不透明");
        CHECK(px(11,11)[3] == 255 && px(13,13)[3] == 255, "大步距画线路径中间像素应不透明");
    }

    /* 7c. 性能参考：4K 宽度快速甩动一笔（r=20，步距 10 → 墨点减少约 10 倍） */
    {
        setup_canvas(3840, 2160);
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        DrawLine(0, 1080, 3839, 1080, 20, RGB(255,0,0), 255);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        long us = (t1.tv_sec - t0.tv_sec) * 1000000L + (t1.tv_nsec - t0.tv_nsec) / 1000;
        printf("参考: 3840px 一笔 (r=20) 耗时 %ld µs（大步距后约 386 个墨点）\n", us);
    }

    /* 8. 工具栏命中测试 */
    CHECK(HitTestToolbar(CLR_X0 + 1, CLR_Y + 1) == 0, "第1个色块命中失败");
    CHECK(HitTestToolbar(CLR_X0 + (CLR_SIZE+CLR_GAP) + 1, CLR_Y + 1) == 1, "第2个色块命中失败");
    CHECK(HitTestToolbar(CLR_X0 + 5*(CLR_SIZE+CLR_GAP) + 1, CLR_Y + 1) == 5, "第6个色块命中失败");
    CHECK(HitTestToolbar(CLEAR_X + 2, BTN_Y + 2) == 6, "Clear 按钮命中失败");
    CHECK(HitTestToolbar(CLOSE_X + 2, BTN_Y + 2) == 7, "Close 按钮命中失败");
    CHECK(HitTestToolbar(2, 2) == -1, "空白区应返回 -1");
    CHECK(HitTestToolbar(CLR_X0 + 1, CLR_Y - 1) == -1, "色块上边缘外应返回 -1");

    printf("=== region.c: 局部失效矩形 ===\n");

    /* 8b. InvalidateSelRect：失效矩形 = (旧选区 ∪ 新选区) 外扩 2px */
    {
        RegionState rs2;
        memset(&rs2, 0, sizeof(rs2));
        rs2.start.x = 100; rs2.start.y = 50;
        rs2.current.x = 200; rs2.current.y = 150;
        RECT got;

        fake_clear_invalidate();
        InvalidateSelRect((HWND)0x1, &rs2);
        /* prev 初始为 0 → 首次失效会包含原点，属无害的多失效一小块区域 */
        CHECK(fake_last_invalidate(&got) &&
              got.left == -2 && got.top == -2 && got.right == 202 && got.bottom == 152,
              "首次失效矩形应为 (-2,-2,202,152)（%d,%d,%d,%d）", got.left, got.top, got.right, got.bottom);
        CHECK(rs2.prev.left == 100 && rs2.prev.top == 50 &&
              rs2.prev.right == 200 && rs2.prev.bottom == 150,
              "prev 应更新为当前选区");

        /* 选区扩大：失效矩形须包含旧选区（旧边框要被擦除） */
        rs2.current.x = 300; rs2.current.y = 200;
        InvalidateSelRect((HWND)0x1, &rs2);
        CHECK(fake_last_invalidate(&got) &&
              got.left == 98 && got.top == 48 && got.right == 302 && got.bottom == 202,
              "选区扩大时失效矩形应包含旧选区（%d,%d,%d,%d）", got.left, got.top, got.right, got.bottom);

        /* 选区缩小：同样须包含旧选区，否则旧边框残留在屏幕上 */
        rs2.current.x = 150; rs2.current.y = 120;
        InvalidateSelRect((HWND)0x1, &rs2);
        CHECK(fake_last_invalidate(&got) &&
              got.left == 98 && got.top == 48 && got.right == 302 && got.bottom == 202,
              "选区缩小时失效矩形应包含旧选区（%d,%d,%d,%d）", got.left, got.top, got.right, got.bottom);
        CHECK(rs2.prev.right == 150 && rs2.prev.bottom == 120, "缩小时后 prev 应更新为新选区");
    }

    printf("=== 防重入 guard ===\n");

    /* 9. 选区：窗口创建失败路径应复位标志 */
    {
        RECT out;
        fake_clear_foreground();
        BOOL ok = RunRegionSelection((HINSTANCE)0x1, &out);
        CHECK(ok == FALSE, "窗口创建失败应返回 FALSE");
        CHECK(s_regionActive == FALSE && s_regionHwnd == NULL,
              "失败路径应复位防重入标志（active=%d）", s_regionActive);
    }

    /* 10. 选区重入：已有选区时再次触发应聚焦并返回 FALSE */
    {
        s_regionActive = TRUE;            /* 白盒：模拟选区进行中 */
        s_regionHwnd = (HWND)0x7;
        fake_clear_foreground();
        RECT out = {0};
        BOOL ok = RunRegionSelection((HINSTANCE)0x1, &out);
        CHECK(ok == FALSE, "选区重入应返回 FALSE");
        CHECK(fake_last_foreground() == (HWND)0x7, "选区重入应聚焦已有选区窗口");
        CHECK(s_regionActive == TRUE, "选区 guard 不应改动进行中状态");
        s_regionActive = FALSE;
        s_regionHwnd = NULL;
    }

    /* 11. 画板重入：已有画布时再次触发应聚焦且不重置状态 */
    {
        setup_canvas(1, 1);               /* 哨兵画布（guard 路径不应触碰） */
        g_draw.hwndCanvas = (HWND)0x1;
        g_draw.pen_size = 7;              /* 哨兵：guard 应提前返回，不 memset */
        fake_clear_foreground();
        RunDrawingMode((HINSTANCE)0x2);
        CHECK(fake_last_foreground() == (HWND)0x1, "画板重入应聚焦已有画布");
        CHECK(g_draw.pen_size == 7, "画板重入 guard 不应 memset 状态（pen_size=%d）", g_draw.pen_size);
        free(g_draw.canvas_bits);
        g_draw.canvas_bits = NULL;
        memset(&g_draw, 0, sizeof(g_draw));
    }

    /* 12. 设置对话框重入 */
    {
        s_settingsHwnd = (HWND)0x9;
        g_changed = FALSE;
        fake_clear_foreground();
        BOOL changed = ShowSettingsDialog((HINSTANCE)0x1);
        CHECK(changed == FALSE, "设置重入应返回 FALSE（无改动）");
        CHECK(g_changed == FALSE, "设置重入不应改动 g_changed");
        CHECK(fake_last_foreground() == (HWND)0x9, "设置重入应聚焦已有对话框");
        s_settingsHwnd = NULL;
    }

    /* 13. 关于对话框重入 */
    {
        s_aboutHwnd = (HWND)0xB;
        fake_clear_foreground();
        ShowAboutDialog((HINSTANCE)0x1);
        CHECK(fake_last_foreground() == (HWND)0xB, "关于重入应聚焦已有对话框");
        s_aboutHwnd = NULL;
    }

    printf("=== hotkey.c: 打包/格式化/注册表 ===\n");

    /* 14. PackHotkey / UnpackHotkey 往返 */
    {
        DWORD pk = PackHotkey(MOD_CONTROL | MOD_SHIFT, 'S');
        UINT m2 = 0, v2 = 0;
        UnpackHotkey(pk, &m2, &v2);
        CHECK(m2 == (MOD_CONTROL | MOD_SHIFT) && v2 == 'S', "Pack/Unpack 往返失败");
        UnpackHotkey(PackHotkey(MOD_ALT, VK_F11), &m2, &v2);
        CHECK(m2 == MOD_ALT && v2 == VK_F11, "Pack/Unpack 往返失败 (Alt+F11)");
    }

    /* 15. FormatHotkey（GetKeyNameText 为空 → 走回退分支） */
    {
        wchar_t buf[48];
        fake_set_keynames(0);
        FormatHotkey(MOD_CONTROL | MOD_SHIFT, 'S', buf, 48);
        CHECK(wcscmp(buf, L"Ctrl+Shift+S") == 0, "期望 Ctrl+Shift+S，实际 %ls", buf);
        FormatHotkey(MOD_ALT, VK_F5, buf, 48);
        CHECK(wcscmp(buf, L"Alt+F5") == 0, "期望 Alt+F5，实际 %ls", buf);
        FormatHotkey(0, VK_OEM_MINUS, buf, 48);
        CHECK(wcscmp(buf, L"-") == 0, "期望 -，实际 %ls", buf);
        FormatHotkey(MOD_CONTROL, 0xFF, buf, 48);
        CHECK(wcscmp(buf, L"Ctrl+VK_255") == 0, "期望 Ctrl+VK_255，实际 %ls", buf);
        FormatHotkey(MOD_SHIFT, '7', buf, 48);
        CHECK(wcscmp(buf, L"Shift+7") == 0, "期望 Shift+7，实际 %ls", buf);
        /* 缓冲区截断安全 */
        FormatHotkey(MOD_CONTROL | MOD_SHIFT | MOD_ALT, 'S', buf, 6);
        CHECK(wcslen(buf) <= 5 && buf[5] == 0, "小缓冲区截断失败");

        /* GetKeyNameText 路径 */
        fake_set_keynames(1);
        FormatHotkey(MOD_CONTROL | MOD_SHIFT, 'S', buf, 48);
        CHECK(wcscmp(buf, L"Ctrl+Shift+S") == 0, "GetKeyNameText 路径期望 Ctrl+Shift+S，实际 %ls", buf);
        FormatHotkey(0, VK_LEFT, buf, 48);
        CHECK(wcscmp(buf, L"Left") == 0, "期望 Left，实际 %ls", buf);
        fake_set_keynames(0);
    }

    /* 16. 注册表读写往返（内存假注册表） */
    {
        UINT mods[HK_COUNT], vks[HK_COUNT];
        fake_registry_clear();
        LoadHotkeys(mods, vks);
        CHECK(mods[0] == (MOD_CONTROL|MOD_SHIFT) && vks[0] == 'S', "空注册表应回退默认值 Pin");
        CHECK(mods[3] == (MOD_CONTROL|MOD_SHIFT) && vks[3] == 'B', "空注册表应回退默认值 Drawing");

        SaveHotkey(L"Pin", MOD_CONTROL | MOD_ALT, 'Q');
        LoadHotkeys(mods, vks);
        CHECK(mods[0] == (MOD_CONTROL|MOD_ALT) && vks[0] == 'Q', "SaveHotkey 后读取不一致");
        CHECK(mods[1] == (MOD_CONTROL|MOD_SHIFT) && vks[1] == 'A', "未保存项应保持默认 PinCopy");

        ResetHotkeys();
        LoadHotkeys(mods, vks);
        CHECK(mods[0] == (MOD_CONTROL|MOD_SHIFT) && vks[0] == 'S', "ResetHotkeys 失败");
        CHECK(fake_registry_count() == 4, "Reset 应写入 4 项，实际 %d", fake_registry_count());
    }

    free(g_draw.canvas_bits); /* 释放最后一块画布，保证 ASan 无泄漏 */

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
