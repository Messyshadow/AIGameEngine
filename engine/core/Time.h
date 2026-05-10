#pragma once

#include <chrono>

namespace AIForge {

/// @ai_summary 时间管理：每帧 Tick 一次，提供 deltaTime/FPS/总时间。
/// @ai_summary 由 App 主循环驱动；通常无需手动构造。
/// @ai_example
///   while (app.Running()) {
///       app.Tick();   // 内部调用 Time::Tick
///       float dt = app.GetTime()->DeltaTime();
///       // 用 dt 推进游戏逻辑
///   }
/// @ai_related App
class Time {
public:
    Time();

    /// @ai_summary 推进一帧；记录上一帧到当前的耗时。
    void Tick();

    /// @ai_summary 上一帧到当前帧的间隔（秒）。被 maxDeltaTime 截断。
    float DeltaTime() const { return m_deltaTime; }

    /// @ai_summary 应用启动到当前的累计时间（秒）。
    float TotalTime() const { return m_totalTime; }

    /// @ai_summary 平滑后的每秒帧数（约每 0.5 秒更新一次）。
    float FPS() const { return m_fps; }

    /// @ai_summary 已经渲染的帧数。
    int FrameCount() const { return m_frameCount; }

    /// @ai_summary deltaTime 的上限（秒）；防止"卡顿一下"导致物理大跳。默认 0.1s。
    void SetMaxDeltaTime(float seconds) { m_maxDeltaTime = seconds; }

private:
    using Clock = std::chrono::steady_clock;
    Clock::time_point m_startTime;
    Clock::time_point m_lastTick;

    float m_deltaTime = 0.0f;
    float m_totalTime = 0.0f;
    float m_maxDeltaTime = 0.1f;

    float m_fps = 0.0f;
    int m_frameCount = 0;
    int m_fpsAccumFrames = 0;
    float m_fpsAccumTime = 0.0f;
};

}  // namespace AIForge
