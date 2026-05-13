// =============================================================================
// AIForge Engine — Chapter 04: 2D Sprite Batcher + Camera2D
// =============================================================================
// 本章 demo:5000 个软圆形 sprite 在屏幕里弹来弹去,锁满刷新率。
//
// 你看到的:
//   - 5000 个半透明彩色圆形,各自有随机速度、颜色、大小
//   - 它们在 1280x720 的世界框里弹来弹去(撞边反弹)
//   - **窗口标题栏一直显示 draw_calls=1** — 这是本章的灵魂:单次 GPU 调用画完所有
//   - WASD 平移摄像机,Q/E 缩小放大,R 重置
//
// 这一章证明的事:
//   ✅ AIForge 已经具备做"类 Vampire Survivors"游戏的渲染基础
//   ✅ 批渲染让 CPU 几乎不工作(看任务管理器,CPU 占用 < 10%)
//   ✅ 性能瓶颈在 GPU 填充率,不在 draw call 数
//
// 程序化纹理(无 PNG 依赖):
//   - 64x64 RGBA,中心 alpha=1.0,边缘渐变到 0.0,形成"软圆"
//   - Ch 05 起会用 stb_image 加载真实 PNG
// =============================================================================

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>

#include "engine/core/App.h"
#include "engine/render/Camera2D.h"
#include "engine/render/SpriteBatcher.h"
#include "engine/render/Texture.h"

constexpr int   kSpriteCount     = 5000;
constexpr float kWorldHalfWidth  = 640.0f;   // 与 1280 viewport 匹配
constexpr float kWorldHalfHeight = 360.0f;   // 与 720 viewport 匹配

struct Sprite {
    AIForge::Vec2 pos;
    AIForge::Vec2 vel;
    AIForge::Vec2 size;
    AIForge::Vec4 color;
    float rotation = 0.0f;
    float angularVel = 0.0f;
};

// -----------------------------------------------------------------------------
// 程序化纹理:64x64 的软圆(白色 + alpha 渐变)
// -----------------------------------------------------------------------------
static std::vector<uint8_t> MakeSoftCircleRGBA(int size) {
    std::vector<uint8_t> px(static_cast<size_t>(size) * size * 4, 0);
    float r = size * 0.5f;
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float dx = (x + 0.5f - r) / r;
            float dy = (y + 0.5f - r) / r;
            float d  = std::sqrt(dx * dx + dy * dy);
            // alpha = 1 在 d<0.7, 渐变到 0 在 d>=1
            float a = 1.0f - std::clamp((d - 0.7f) / 0.3f, 0.0f, 1.0f);
            size_t idx = (static_cast<size_t>(y) * size + x) * 4;
            px[idx + 0] = 255;
            px[idx + 1] = 255;
            px[idx + 2] = 255;
            px[idx + 3] = static_cast<uint8_t>(a * 255);
        }
    }
    return px;
}

int main(int /*argc*/, char* /*argv*/[]) {
    // ----------------------------------- App 启动 ---
    auto app = AIForge::App::Create();
    if (!app) return 1;

    AIForge::Window::Config wc;
    wc.title  = "AIForge Ch 04 - 2D Sprite Batcher";
    wc.width  = 1280;
    wc.height = 720;
    wc.vsync  = true;
    app->SetWindowConfig(wc);

    if (!app->Init()) {
        std::fprintf(stderr, "[Ch04] App::Init failed\n");
        return 1;
    }
    app->SetClearColor(0.05f, 0.05f, 0.08f, 1.0f);   // 近黑(让 sprite 更亮)
    app->SetShowFPSInTitle(false);

    // ----------------------------------- 纹理 + Batcher + Camera ---
    AIForge::Texture texture;
    auto pixels = MakeSoftCircleRGBA(64);
    if (!texture.CreateFromRGBA(64, 64, pixels.data())) return 1;

    AIForge::SpriteBatcher batcher;
    if (!batcher.Init(kSpriteCount + 100)) return 1;

    AIForge::Camera2D camera;
    camera.SetViewport(1280, 720);

    // ----------------------------------- 生成 5000 个 sprite ---
    std::mt19937 rng(20260514u);
    std::uniform_real_distribution<float> distX(-kWorldHalfWidth + 30, kWorldHalfWidth - 30);
    std::uniform_real_distribution<float> distY(-kWorldHalfHeight + 30, kWorldHalfHeight - 30);
    std::uniform_real_distribution<float> distV(-200.0f, 200.0f);
    std::uniform_real_distribution<float> distSize(10.0f, 40.0f);
    std::uniform_real_distribution<float> distHue(0.0f, 1.0f);
    std::uniform_real_distribution<float> distRot(-2.0f, 2.0f);

    std::vector<Sprite> sprites(kSpriteCount);
    for (auto& s : sprites) {
        s.pos = {distX(rng), distY(rng)};
        s.vel = {distV(rng), distV(rng)};
        float sz = distSize(rng);
        s.size = {sz, sz};
        // HSV → RGB(简单彩虹)
        float h = distHue(rng) * 6.0f;
        int   hi = static_cast<int>(h);
        float f  = h - hi;
        float r = 0, g = 0, b = 0;
        switch (hi) {
            case 0: r = 1; g = f; b = 0; break;
            case 1: r = 1 - f; g = 1; b = 0; break;
            case 2: r = 0; g = 1; b = f; break;
            case 3: r = 0; g = 1 - f; b = 1; break;
            case 4: r = f; g = 0; b = 1; break;
            default:r = 1; g = 0; b = 1 - f; break;
        }
        s.color = {r, g, b, 0.85f};
        s.rotation = 0.0f;
        s.angularVel = distRot(rng);
    }

    std::printf("\n");
    std::printf("==================================================\n");
    std::printf(" AIForge Chapter 04 - 2D Sprite Batcher\n");
    std::printf("==================================================\n");
    std::printf(" %d sprites, target single draw call\n", kSpriteCount);
    std::printf(" Controls:\n");
    std::printf("   WASD      - pan camera\n");
    std::printf("   Q / E     - zoom out / in\n");
    std::printf("   R         - reset camera\n");
    std::printf("   ESC       - quit\n");
    std::printf("==================================================\n\n");

    float titleAccum = 0.0f;

    app->SetUpdateCallback([&](AIForge::App& a, float dt) {
        auto* in = a.GetInput();

        // 1) 摄像机控制
        float panSpeed = 400.0f / camera.zoom;
        if (in->IsKeyDown(AIForge::K_W)) camera.position.y += panSpeed * dt;
        if (in->IsKeyDown(AIForge::K_S)) camera.position.y -= panSpeed * dt;
        if (in->IsKeyDown(AIForge::K_A)) camera.position.x -= panSpeed * dt;
        if (in->IsKeyDown(AIForge::K_D)) camera.position.x += panSpeed * dt;
        if (in->IsKeyDown(AIForge::K_Q)) camera.zoom *= std::pow(0.4f, dt);
        if (in->IsKeyDown(AIForge::K_E)) camera.zoom *= std::pow(2.5f, dt);
        if (in->IsKeyPressed(AIForge::K_R)) {
            camera.position = {0, 0};
            camera.zoom = 1.0f;
            camera.rotation = 0.0f;
        }
        camera.zoom = std::clamp(camera.zoom, 0.1f, 5.0f);

        // 同步窗口大小到摄像机(窗口可拖)
        camera.SetViewport(a.GetWindow()->GetWidth(),
                            a.GetWindow()->GetHeight());

        // 2) sprite 物理:边界反弹
        for (auto& s : sprites) {
            s.pos.x += s.vel.x * dt;
            s.pos.y += s.vel.y * dt;
            s.rotation += s.angularVel * dt;

            if (s.pos.x < -kWorldHalfWidth)  { s.pos.x = -kWorldHalfWidth;  s.vel.x = -s.vel.x; }
            if (s.pos.x >  kWorldHalfWidth)  { s.pos.x =  kWorldHalfWidth;  s.vel.x = -s.vel.x; }
            if (s.pos.y < -kWorldHalfHeight) { s.pos.y = -kWorldHalfHeight; s.vel.y = -s.vel.y; }
            if (s.pos.y >  kWorldHalfHeight) { s.pos.y =  kWorldHalfHeight; s.vel.y = -s.vel.y; }
        }

        // 3) 批渲染:一次 Begin → 5000 次 Submit → End → 单 draw call
        batcher.Begin(camera, texture);
        for (auto& s : sprites) {
            batcher.Submit(s.pos, s.size, s.color, s.rotation);
        }
        batcher.End();

        // 4) 标题栏统计
        titleAccum += dt;
        if (titleAccum >= 0.25f) {
            titleAccum = 0.0f;
            char buf[256];
            std::snprintf(buf, sizeof(buf),
                "AIForge Ch 04 | sprites=%d | draw_calls=%d | zoom=%.2f | %.0f FPS",
                batcher.GetSpriteCount(), batcher.GetDrawCallCount(),
                camera.zoom, a.GetTime()->FPS());
            a.GetWindow()->SetTitle(buf);
        }
    });

    app->Run();
    app->Shutdown();
    std::printf("\n[Ch04] clean exit. Bye!\n");
    return 0;
}
