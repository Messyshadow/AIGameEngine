#pragma once

#include <SDL3/SDL_video.h>

#include <string>

namespace AIForge {

/// @ai_summary 应用窗口（基于 SDL3 + OpenGL 4.5 Core）。由 App 创建管理。
/// @ai_summary 你通常不直接用 Window，配置写在 engine.json 的 "window" 下。
/// @ai_example
///   Window::Config cfg;
///   cfg.title = "My Game";
///   cfg.width = 1280; cfg.height = 720;
///   Window window;
///   window.Init(cfg);
///   while (!window.ShouldClose()) {
///       // ... draw
///       window.SwapBuffers();
///   }
/// @ai_related App, Input
class Window {
public:
    /// @ai_summary 窗口配置；从 engine.json 加载或代码里直接填。
    struct Config {
        std::string title = "AIForge App";
        int width = 1280;
        int height = 720;
        bool fullscreen = false;
        bool vsync = true;
    };

    Window();
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    /// @ai_summary 初始化 SDL3 + 创建窗口 + 创建 OpenGL 4.5 Core 上下文 + 加载 glad。
    /// @ai_summary 失败时打印错误并返回 false（不抛异常）。
    bool Init(const Config& cfg);

    /// @ai_summary 销毁窗口与上下文；可重复调用。
    void Shutdown();

    /// @ai_summary 交换前后缓冲（每帧最后调用）。
    void SwapBuffers();

    bool ShouldClose() const { return m_shouldClose; }
    void RequestClose() { m_shouldClose = true; }

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    void SetTitle(const std::string& title);

    SDL_Window* GetSDLWindow() const { return m_window; }
    SDL_GLContext GetGLContext() const { return m_glContext; }

private:
    SDL_Window* m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;
    int m_width = 0;
    int m_height = 0;
    bool m_shouldClose = false;
    bool m_initialized = false;
};

}  // namespace AIForge
