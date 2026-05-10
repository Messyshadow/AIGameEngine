#pragma once

#include <functional>
#include <memory>
#include <string>

#include "ECS.h"
#include "Input.h"
#include "ResourceManager.h"
#include "Time.h"
#include "Window.h"

namespace AIForge {

class CommandRegistry;

/// @ai_summary App 是引擎入口：拥有 Window/Input/Time/World/ResourceManager/CommandRegistry。
/// @ai_summary 创建方式：AIForge::App::Create() 或 Create("data/config/engine.json")。
/// @ai_summary 主循环：app->Run() 阻塞直到窗口关闭；或自己每帧 app->Tick()。
/// @ai_example
///   auto app = AIForge::App::Create("data/config/engine.json");
///   if (!app || !app->Init()) return 1;
///   app->SetUpdateCallback([](AIForge::App& a, float dt) {
///       // 每帧逻辑
///   });
///   app->Run();
/// @ai_related Window, World, CommandRegistry, AIContext
class App {
public:
    using UpdateFn = std::function<void(App&, float)>;

    /// @ai_summary 构造一个未初始化的 App，使用默认窗口配置。
    static std::unique_ptr<App> Create();

    /// @ai_summary 从 engine.json 构造 App（自动读取窗口与引擎参数）。
    /// @ai_summary 文件读取失败时，返回带默认值的 App，并打印警告。
    static std::unique_ptr<App> Create(const std::string& configPath);

    App();
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    /// @ai_summary 初始化窗口/OpenGL/输入/时间/世界/资源/命令系统。失败时返回 false。
    bool Init();

    /// @ai_summary 关闭并释放所有资源；可重复调用。
    void Shutdown();

    /// @ai_summary 推进一帧（事件/输入/时间/世界/渲染清屏/SwapBuffers）。
    void Tick();

    /// @ai_summary 主循环：反复 Tick 直到 ShouldClose。
    void Run();

    /// @ai_summary 申请退出（下一帧 ShouldClose 为 true）。
    void RequestClose();
    bool ShouldClose() const;

    /// @ai_summary 执行一条文本命令，例如 "spawn player at 0,0,0"。
    /// @ai_params cmd 命令文本（非空）
    /// @ai_params errMsg 失败时输出错误信息
    /// @ai_example
    ///   std::string err;
    ///   if (!app.Execute("spawn zombie at 5,0,0", err)) std::printf("%s\n", err.c_str());
    /// @ai_related CommandRegistry, CommandParser
    bool Execute(const std::string& cmd, std::string& errMsg);
    bool Execute(const std::string& cmd);

    /// @ai_summary 注册每帧逻辑回调（在 World::Update 之后调用）。
    void SetUpdateCallback(UpdateFn fn) { m_onUpdate = std::move(fn); }

    /// @ai_summary 设置每帧 glClearColor。默认 (0.10, 0.15, 0.25, 1.0)。
    void SetClearColor(float r, float g, float b, float a);

    /// @ai_summary 是否按 ESC 自动 RequestClose。默认 true。
    void SetEscQuits(bool enable) { m_escQuits = enable; }

    /// @ai_summary 是否在窗口标题追加 FPS。默认 true。
    void SetShowFPSInTitle(bool enable);

    /// @ai_summary 设置窗口配置（必须在 Init 之前调用）。
    void SetWindowConfig(const Window::Config& cfg) { m_windowCfg = cfg; }
    const Window::Config& GetWindowConfig() const { return m_windowCfg; }

    Window* GetWindow() { return m_window.get(); }
    Input* GetInput() { return m_input.get(); }
    Time* GetTime() { return m_time.get(); }
    World* GetWorld() { return m_world.get(); }
    ResourceManager* GetResources() { return m_resources.get(); }
    CommandRegistry* GetCommands() { return m_commands.get(); }

private:
    void RegisterBuiltinCommands();
    void PollEvents();
    void RenderClear();
    void UpdateTitle();

    Window::Config m_windowCfg;
    std::string m_baseTitle;
    float m_clearColor[4] = {0.10f, 0.15f, 0.25f, 1.0f};
    bool m_initialized = false;
    bool m_escQuits = true;
    bool m_showFPS = true;

    std::unique_ptr<Window> m_window;
    std::unique_ptr<Input> m_input;
    std::unique_ptr<Time> m_time;
    std::unique_ptr<World> m_world;
    std::unique_ptr<ResourceManager> m_resources;
    std::unique_ptr<CommandRegistry> m_commands;

    UpdateFn m_onUpdate;

    float m_titleUpdateAccum = 0.0f;
};

}  // namespace AIForge
