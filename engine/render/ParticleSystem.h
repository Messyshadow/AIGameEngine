#pragma once

#include <vector>

#include "../core/Math.h"

namespace AIForge {

class SpriteBatcher;
class Texture;

/// @ai_summary 单个粒子的数据(AoS 风格,清晰但缓存不太友好;Ch 13 GPU 版改 SoA)。
struct Particle {
    Vec2  pos{0, 0};
    Vec2  vel{0, 0};
    Vec2  accel{0, 0};       // 重力等
    float age  = 0.0f;
    float life = 1.0f;       // 生命周期(秒)
    Vec4  colorStart{1, 1, 1, 1};
    Vec4  colorEnd{1, 1, 1, 0};
    Vec2  sizeStart{8, 8};
    Vec2  sizeEnd{2, 2};
    float rotation = 0.0f;
    float angularVel = 0.0f;
    bool  alive = false;
};

/// @ai_summary 粒子发射器:配置 + 调用 Update(dt) 持续发射并更新所有粒子。
/// @ai_summary 池预分配 max=10000 个粒子,Spawn 时找一个 alive=false 的复用。
/// @ai_example
///   ParticleEmitter em;
///   em.origin     = {0, -20};   // 角色脚下
///   em.rate       = 60.0f;
///   em.lifeMin    = 0.4f; em.lifeMax = 1.2f;
///   em.velAngleDeg = 90;        // 向上发射
///   em.velSpreadDeg = 30;
///   em.velMagMin = 50; em.velMagMax = 150;
///   em.gravity    = {0, -200};  // 下落感
///   em.colorStart = {1.0, 0.7, 0.3, 1.0};   // 黄褐色
///   em.colorEnd   = {0.4, 0.3, 0.1, 0.0};   // 渐隐
///   em.sizeStart  = {6, 6};
///   em.sizeEnd    = {2, 2};
///   em.Init(1000);
///   while (running) {
///       em.SetOrigin(playerPos);
///       em.Update(dt);
///       em.Render(batcher);
///   }
/// @ai_related SpriteBatcher, ParticleSystem
class ParticleEmitter {
public:
    // 发射配置 — 用户直接读写
    Vec2  origin{0, 0};
    bool  emitting = true;
    float rate         = 30.0f;   // 粒子/秒
    float lifeMin      = 0.5f;
    float lifeMax      = 1.5f;
    float velAngleDeg  = 90.0f;   // 主方向(角度,0=右,90=上)
    float velSpreadDeg = 30.0f;   // 扇形半张角
    float velMagMin    = 50.0f;
    float velMagMax    = 150.0f;
    Vec2  gravity{0, -200};
    Vec4  colorStart{1, 1, 1, 1};
    Vec4  colorEnd{1, 1, 1, 0};
    Vec2  sizeStart{8, 8};
    Vec2  sizeEnd{2, 2};

    /// @ai_summary 初始化粒子池容量(后续 Spawn 不超过 max)。
    bool Init(int maxParticles = 5000);

    /// @ai_summary 推进一帧:发射 + 老化 + 物理。
    void Update(float dt);

    /// @ai_summary 提交所有活着的粒子到 SpriteBatcher(需要 Begin 之后调)。
    void Render(SpriteBatcher& batcher) const;

    /// @ai_summary 瞬间爆发 N 个粒子(忽略 rate,用于爆炸效果)。
    void Burst(int count);

    void  SetOrigin(Vec2 p) { origin = p; }
    int   GetAliveCount() const;
    int   GetCapacity()   const { return (int)m_pool.size(); }

private:
    void SpawnOne();

    std::vector<Particle> m_pool;
    float                 m_emitAccum = 0.0f;
};

}  // namespace AIForge
