#include "SpriteAnimation.h"

namespace AIForge {

void SpriteAnimation::Tick(float dt) {
    if (frames.empty()) return;
    m_time += dt;
}

int SpriteAnimation::GetSequenceIndex() const {
    if (frames.empty()) return 0;
    float frameDuration = 1.0f / (fps > 0 ? fps : 8.0f);
    int idx = (int)(m_time / frameDuration);
    if (loop) {
        int n = (int)frames.size();
        idx = ((idx % n) + n) % n;
    } else {
        int last = (int)frames.size() - 1;
        if (idx > last) idx = last;
        if (idx < 0)    idx = 0;
    }
    return idx;
}

int SpriteAnimation::GetCurrentFrameIndex() const {
    if (frames.empty()) return 0;
    return frames[GetSequenceIndex()];
}

void SpriteAnimation::Reset() { m_time = 0.0f; }

bool SpriteAnimation::IsFinished() const {
    if (loop || frames.empty()) return false;
    float total = frames.size() / (fps > 0 ? fps : 8.0f);
    return m_time >= total;
}

}  // namespace AIForge
