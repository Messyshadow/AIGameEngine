#include "Time.h"

namespace AIForge {

Time::Time() {
    m_startTime = Clock::now();
    m_lastTick = m_startTime;
}

void Time::Tick() {
    auto now = Clock::now();
    std::chrono::duration<float> delta = now - m_lastTick;
    std::chrono::duration<float> total = now - m_startTime;
    m_lastTick = now;

    float dt = delta.count();
    if (dt > m_maxDeltaTime) dt = m_maxDeltaTime;
    if (dt < 0.0f) dt = 0.0f;

    m_deltaTime = dt;
    m_totalTime = total.count();
    ++m_frameCount;

    m_fpsAccumFrames++;
    m_fpsAccumTime += dt;
    if (m_fpsAccumTime >= 0.5f) {
        m_fps = m_fpsAccumFrames / m_fpsAccumTime;
        m_fpsAccumFrames = 0;
        m_fpsAccumTime = 0.0f;
    }
}

}  // namespace AIForge
