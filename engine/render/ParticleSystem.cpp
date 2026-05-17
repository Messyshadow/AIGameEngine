#include "ParticleSystem.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "SpriteBatcher.h"

namespace AIForge {

namespace {
inline float Rand01() { return (float)std::rand() / (float)RAND_MAX; }
inline float RandRange(float a, float b) { return a + (b - a) * Rand01(); }
}  // namespace

bool ParticleEmitter::Init(int maxParticles) {
    if (maxParticles <= 0) maxParticles = 1024;
    m_pool.assign(maxParticles, Particle{});
    m_emitAccum = 0.0f;
    return true;
}

void ParticleEmitter::SpawnOne() {
    // 找第一个 alive=false 的槽位复用
    for (auto& p : m_pool) {
        if (p.alive) continue;
        p.alive = true;
        p.pos   = origin;
        float angle = (velAngleDeg + RandRange(-velSpreadDeg, velSpreadDeg)) *
                       3.14159265f / 180.0f;
        float speed = RandRange(velMagMin, velMagMax);
        p.vel       = {std::cos(angle) * speed, std::sin(angle) * speed};
        p.accel     = gravity;
        p.age       = 0.0f;
        p.life      = RandRange(lifeMin, lifeMax);
        p.colorStart = colorStart;
        p.colorEnd   = colorEnd;
        p.sizeStart  = sizeStart;
        p.sizeEnd    = sizeEnd;
        p.rotation   = RandRange(0.0f, 6.28f);
        p.angularVel = RandRange(-3.0f, 3.0f);
        return;
    }
    // 池满了静默丢弃(避免每秒打几千行 warn)
}

void ParticleEmitter::Burst(int count) {
    for (int i = 0; i < count; ++i) SpawnOne();
}

void ParticleEmitter::Update(float dt) {
    if (dt < 0) dt = 0;

    // 1) 发射
    if (emitting && rate > 0.0f) {
        m_emitAccum += dt * rate;
        int toSpawn = (int)m_emitAccum;
        if (toSpawn > 0) {
            m_emitAccum -= toSpawn;
            for (int i = 0; i < toSpawn; ++i) SpawnOne();
        }
    }

    // 2) 推进所有活粒子
    for (auto& p : m_pool) {
        if (!p.alive) continue;
        p.age += dt;
        if (p.age >= p.life) {
            p.alive = false;
            continue;
        }
        p.vel.x += p.accel.x * dt;
        p.vel.y += p.accel.y * dt;
        p.pos.x += p.vel.x * dt;
        p.pos.y += p.vel.y * dt;
        p.rotation += p.angularVel * dt;
    }
}

void ParticleEmitter::Render(SpriteBatcher& batcher) const {
    for (auto& p : m_pool) {
        if (!p.alive) continue;
        float t = p.age / p.life;          // 0 -> 1 over lifetime
        Vec4 c;
        c.x = p.colorStart.x + (p.colorEnd.x - p.colorStart.x) * t;
        c.y = p.colorStart.y + (p.colorEnd.y - p.colorStart.y) * t;
        c.z = p.colorStart.z + (p.colorEnd.z - p.colorStart.z) * t;
        c.w = p.colorStart.w + (p.colorEnd.w - p.colorStart.w) * t;
        Vec2 sz;
        sz.x = p.sizeStart.x + (p.sizeEnd.x - p.sizeStart.x) * t;
        sz.y = p.sizeStart.y + (p.sizeEnd.y - p.sizeStart.y) * t;
        batcher.Submit(p.pos, sz, c, p.rotation);
    }
}

int ParticleEmitter::GetAliveCount() const {
    int n = 0;
    for (auto& p : m_pool) if (p.alive) ++n;
    return n;
}

}  // namespace AIForge
