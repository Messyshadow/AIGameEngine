#pragma once

#include <vector>

namespace AIForge {

/// @ai_summary 帧动画播放器:由"帧索引序列 + 每秒帧数 + 是否循环"组成。
/// @ai_summary 每帧 Tick(dt) 推进时间,GetCurrentFrameIndex() 取当前帧。
/// @ai_example
///   SpriteAnimation walkAnim;
///   walkAnim.frames = {0, 1, 2, 3, 4, 5, 6, 7};   // 第 0-7 帧
///   walkAnim.fps    = 12.0f;
///   walkAnim.loop   = true;
///   // 主循环里:
///   walkAnim.Tick(dt);
///   int curFrame = walkAnim.GetCurrentFrameIndex();
/// @ai_related SpriteSheet, SpriteBatcher
class SpriteAnimation {
public:
    std::vector<int> frames;  // 在 SpriteSheet 中的帧索引序列
    float            fps   = 8.0f;
    bool             loop  = true;

    /// @ai_summary 推进时间;到末尾如不 loop 则停在最后一帧。
    void Tick(float dt);

    /// @ai_summary 取当前应该显示的帧索引(SpriteSheet 索引)。
    int GetCurrentFrameIndex() const;

    /// @ai_summary 回到第 0 帧,重置时间累加器。
    void Reset();

    /// @ai_summary 是否播放完(仅非 loop 时有意义)。
    bool IsFinished() const;

    /// @ai_summary 当前在动画序列中的逻辑帧号(从 0 起)。
    int GetSequenceIndex() const;

private:
    float m_time = 0.0f;
};

}  // namespace AIForge
