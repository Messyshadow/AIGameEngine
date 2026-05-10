// =============================================================================
// AIForge Engine — Chapter 01: Hello, Engine
// =============================================================================
// 本章交付的最小可运行示例。
//
// 你看到的:
//   - 一扇 1280x720 的可调整大小窗口
//   - 深蓝色背景(R=0.10, G=0.15, B=0.25)
//   - 标题栏每 0.25 秒更新一次,显示当前 FPS
//   - 按 ESC 即可关闭窗口退出
//
// 你看不到但确实在跑的:
//   - SDL3 视频子系统初始化
//   - OpenGL 4.5 Core Profile 上下文创建 + glad 加载所有函数指针
//   - 双缓冲 + VSync(60 FPS 稳定)
//   - SDL 事件循环:把键盘/鼠标/退出事件分派到 AIForge::Input
//   - AIForge::App 主循环:NewFrame → PollEvents → Tick → Clear → Swap
//
// 整个 main 函数极短,因为引擎自身做了所有重活。
// =============================================================================

#include <cstdio>

#include "engine/core/App.h"

int main(int /*argc*/, char* /*argv*/[]) {
    // 1. 创建 App(默认配置:1280x720, vsync, 标题 "AIForge App")
    auto app = AIForge::App::Create();
    if (!app) {
        std::fprintf(stderr, "[Ch01] App::Create returned null\n");
        return 1;
    }

    // 2. 自定义本章窗口标题(不改默认配置文件,代码里直接覆盖)
    AIForge::Window::Config wc;
    wc.title      = "AIForge Ch 01 - Hello, Engine";
    wc.width      = 1280;
    wc.height     = 720;
    wc.fullscreen = false;
    wc.vsync      = true;
    app->SetWindowConfig(wc);

    // 3. 初始化(创建 SDL3 窗口 + GL 4.5 Core 上下文 + 加载 glad)
    if (!app->Init()) {
        std::fprintf(stderr, "[Ch01] App::Init failed\n");
        return 1;
    }

    // 4. 经典深蓝
    app->SetClearColor(0.10f, 0.15f, 0.25f, 1.0f);

    std::printf("\n");
    std::printf("=== AIForge Ch 01 — Hello, Engine ===\n");
    std::printf("Press ESC to quit. Title bar shows live FPS.\n");
    std::printf("=====================================\n\n");

    // 5. 主循环:阻塞直到窗口关闭(ESC 也会触发 RequestClose)
    app->Run();

    // 6. RAII 析构会调 Shutdown,这里显式调一下让日志清晰
    app->Shutdown();

    std::printf("\n[Ch01] clean exit. Bye!\n");
    return 0;
}
