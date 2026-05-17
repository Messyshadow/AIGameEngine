// =============================================================================
// AIForge Engine — Chapter 06: 2D Lighting + Post-processing (Bloom)
// =============================================================================
// 本章 demo:一个黑暗的瓦片世界,机甲角色举着火把,场景散布彩色灯,带 Bloom 溢光。
//
// 4-pass 渲染管线:
//   Pass 1  场景  -> sceneFBO   (瓦片 + 角色 + 火星粒子,Alpha 混合)
//   Pass 2  光照  -> lightFBO   (清成环境光,additive 叠加每个光源)
//   Pass 3  合成              (sceneFBO × lightFBO)
//   Pass 4  Bloom + tonemap -> 屏幕
//
// 控制:
//   WASD   移动角色(火把光跟随)
//   L      开/关光照 —— 对比"全亮平面" vs "黑暗+光"(本章精髓)
//   B      开/关 Bloom —— 对比有无溢光
//   ESC    退出
//
// 本 demo 会加载 robot3_idle.png,因此同时验证了 Ch 05 的贴图加载修复。
// =============================================================================

#include <glad/gl.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <vector>

#include "engine/core/App.h"
#include "engine/render/Camera2D.h"
#include "engine/render/Framebuffer.h"
#include "engine/render/ParticleSystem.h"
#include "engine/render/PostProcess.h"
#include "engine/render/SpriteAnimation.h"
#include "engine/render/SpriteBatcher.h"
#include "engine/render/SpriteSheet.h"
#include "engine/render/Texture.h"
#include "engine/render/Tilemap.h"

// 程序化软圆(粒子用):白 RGB + 径向 alpha
static std::vector<uint8_t> MakeSoftCircle(int size, float coreRatio) {
    std::vector<uint8_t> px((size_t)size * size * 4, 0);
    float r = size * 0.5f;
    for (int y = 0; y < size; ++y)
        for (int x = 0; x < size; ++x) {
            float dx = (x + 0.5f - r) / r, dy = (y + 0.5f - r) / r;
            float d  = std::sqrt(dx * dx + dy * dy);
            float a  = 1.0f - std::clamp((d - coreRatio) / (1.0f - coreRatio),
                                          0.0f, 1.0f);
            size_t i = ((size_t)y * size + x) * 4;
            px[i] = px[i + 1] = px[i + 2] = 255;
            px[i + 3] = (uint8_t)(a * 255);
        }
    return px;
}

// 程序化径向光(光源用):白 RGB + 二次衰减 alpha(中心亮,边缘柔和)
static std::vector<uint8_t> MakeRadialLight(int size) {
    std::vector<uint8_t> px((size_t)size * size * 4, 0);
    float r = size * 0.5f;
    for (int y = 0; y < size; ++y)
        for (int x = 0; x < size; ++x) {
            float dx = (x + 0.5f - r) / r, dy = (y + 0.5f - r) / r;
            float d  = std::sqrt(dx * dx + dy * dy);
            float a  = std::clamp(1.0f - d, 0.0f, 1.0f);
            a = a * a;  // 二次衰减,更像真实光
            size_t i = ((size_t)y * size + x) * 4;
            px[i] = px[i + 1] = px[i + 2] = 255;
            px[i + 3] = (uint8_t)(a * 255);
        }
    return px;
}

struct Light {
    AIForge::Vec2 pos;
    float         radius;
    AIForge::Vec4 color;   // rgb = 光色,a 不用(径向贴图提供 alpha)
};

int main(int /*argc*/, char* /*argv*/[]) {
    auto app = AIForge::App::Create();
    if (!app) return 1;

    AIForge::Window::Config wc;
    wc.title  = "AIForge Ch 06 - 2D Lighting + Bloom";
    wc.width  = 1280;
    wc.height = 720;
    wc.vsync  = true;
    app->SetWindowConfig(wc);
    if (!app->Init()) return 1;
    app->SetShowFPSInTitle(false);

    int winW = 1280, winH = 720;

    // -------------------------------------------------- 素材 -----
    AIForge::Texture idleTex;
    bool charLoaded =
        idleTex.CreateFromFile("data/textures/character/robot3_idle.png",
                               false, false);
    if (!charLoaded) {
        std::fprintf(stderr,
            "[Ch06] WARN: robot3_idle.png missing — character will be a box\n");
    }
    AIForge::SpriteSheet idleSheet;
    AIForge::SpriteAnimation idleAnim;
    if (charLoaded) {
        idleSheet.Set(idleTex, 200, 200);
        idleAnim.frames = {0, 1, 2, 3, 4, 5};
        idleAnim.fps    = 6.0f;
        idleAnim.loop   = true;
    }

    AIForge::Texture whitePixel;
    uint8_t white[4] = {255, 255, 255, 255};
    whitePixel.CreateFromRGBA(1, 1, white);

    auto circlePx = MakeSoftCircle(32, 0.55f);
    AIForge::Texture particleTex;
    particleTex.CreateFromRGBA(32, 32, circlePx.data());

    auto lightPx = MakeRadialLight(256);
    AIForge::Texture lightTex;
    lightTex.CreateFromRGBA(256, 256, lightPx.data());

    // -------------------------------------------------- 地图 -----
    AIForge::Tilemap map;
    map.Init(32, 24, 64);
    map.GenerateProcedural();
    float mapW = (float)(32 * 64), mapH = (float)(24 * 64);

    // -------------------------------------------------- 渲染设施 -----
    AIForge::SpriteBatcher batcher;
    if (!batcher.Init(20000)) return 1;

    AIForge::Camera2D camera;
    camera.SetViewport(winW, winH);
    camera.position = {mapW * 0.5f, mapH * 0.5f};

    AIForge::Framebuffer sceneFBO, lightFBO;
    if (!sceneFBO.Create(winW, winH)) return 1;
    if (!lightFBO.Create(winW, winH)) return 1;

    AIForge::PostProcess post;
    if (!post.Init(winW, winH)) return 1;
    post.SetBloomThreshold(0.55f);
    post.SetBlurIterations(3);

    // -------------------------------------------------- 火星粒子 -----
    AIForge::ParticleEmitter embers;
    embers.rate         = 35.0f;
    embers.lifeMin      = 0.8f;
    embers.lifeMax      = 1.8f;
    embers.velAngleDeg  = 90.0f;
    embers.velSpreadDeg = 35.0f;
    embers.velMagMin    = 20.0f;
    embers.velMagMax    = 70.0f;
    embers.gravity      = {0, 25.0f};   // 微微上浮
    embers.colorStart   = {1.0f, 0.75f, 0.30f, 1.0f};
    embers.colorEnd     = {1.0f, 0.30f, 0.05f, 0.0f};
    embers.sizeStart    = {10, 10};
    embers.sizeEnd      = {2, 2};
    embers.emitting     = true;
    embers.Init(2000);

    // -------------------------------------------------- 光源 -----
    std::vector<Light> staticLights = {
        {{mapW * 0.25f, mapH * 0.30f}, 320.0f, {1.00f, 0.55f, 0.20f, 1}}, // 暖橙
        {{mapW * 0.75f, mapH * 0.28f}, 300.0f, {0.25f, 0.70f, 1.00f, 1}}, // 冷青
        {{mapW * 0.78f, mapH * 0.72f}, 300.0f, {1.00f, 0.30f, 0.85f, 1}}, // 品红
        {{mapW * 0.22f, mapH * 0.74f}, 300.0f, {0.45f, 1.00f, 0.45f, 1}}, // 绿
    };

    AIForge::Vec2 charPos = {mapW * 0.5f, mapH * 0.5f};
    bool lightingOn = true;
    bool bloomOn    = true;
    float titleAccum = 0.0f;

    std::printf("\n==================================================\n");
    std::printf(" AIForge Chapter 06 - 2D Lighting + Bloom\n");
    std::printf("==================================================\n");
    std::printf(" WASD  move character (torch follows)\n");
    std::printf(" L     toggle lighting   B  toggle bloom\n");
    std::printf(" ESC   quit\n");
    std::printf("==================================================\n\n");

    app->SetUpdateCallback([&](AIForge::App& a, float dt) {
        auto* in = a.GetInput();

        // 窗口尺寸变化 -> 重建 FBO
        int w = a.GetWindow()->GetWidth();
        int h = a.GetWindow()->GetHeight();
        if (w != winW || h != winH) {
            winW = w; winH = h;
            sceneFBO.Resize(winW, winH);
            lightFBO.Resize(winW, winH);
            post.Resize(winW, winH);
        }
        camera.SetViewport(winW, winH);

        // 输入
        if (in->IsKeyPressed(AIForge::K_L)) lightingOn = !lightingOn;
        if (in->IsKeyPressed(AIForge::K_B)) bloomOn    = !bloomOn;

        AIForge::Vec2 dir{0, 0};
        if (in->IsKeyDown(AIForge::K_W)) dir.y += 1;
        if (in->IsKeyDown(AIForge::K_S)) dir.y -= 1;
        if (in->IsKeyDown(AIForge::K_A)) dir.x -= 1;
        if (in->IsKeyDown(AIForge::K_D)) dir.x += 1;
        if (dir.x != 0 || dir.y != 0) {
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            charPos.x += dir.x / len * 260.0f * dt;
            charPos.y += dir.y / len * 260.0f * dt;
        }
        charPos.x = std::clamp(charPos.x, 80.0f, mapW - 80.0f);
        charPos.y = std::clamp(charPos.y, 80.0f, mapH - 80.0f);

        idleAnim.Tick(dt);

        // 火星从角色身上升起
        embers.SetOrigin({charPos.x, charPos.y - 10.0f});
        embers.Update(dt);

        // 摄像机平滑跟随
        float k = std::min(1.0f, 5.0f * dt);
        camera.position.x += (charPos.x - camera.position.x) * k;
        camera.position.y += (charPos.y - camera.position.y) * k;

        // 火把光带轻微摇曳
        float flicker = 0.85f + 0.15f * std::sin(a.GetTime()->TotalTime() * 11.0f);

        // ============ Pass 1: 场景 -> sceneFBO ============
        sceneFBO.Bind();
        glClearColor(0.04f, 0.04f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        batcher.Begin(camera, whitePixel, AIForge::SpriteBatcher::BlendMode::Alpha);
        map.Render(batcher, camera);
        batcher.End();

        batcher.Begin(camera, particleTex, AIForge::SpriteBatcher::BlendMode::Alpha);
        embers.Render(batcher);
        batcher.End();

        if (charLoaded) {
            AIForge::Vec4 uv = idleSheet.GetFrameUV(idleAnim.GetCurrentFrameIndex());
            batcher.Begin(camera, idleTex, AIForge::SpriteBatcher::BlendMode::Alpha);
            batcher.SubmitUV(charPos, {190, 190}, {1, 1, 1, 1}, 0.0f, uv);
            batcher.End();
        } else {
            batcher.Begin(camera, whitePixel, AIForge::SpriteBatcher::BlendMode::Alpha);
            batcher.Submit(charPos, {90, 120}, {0.8f, 0.85f, 0.9f, 1.0f});
            batcher.End();
        }

        // ============ Pass 2: 光照 -> lightFBO ============
        lightFBO.Bind();
        // 清成环境光(很暗 — 这样没光的地方几乎黑)
        glClearColor(0.16f, 0.16f, 0.20f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        batcher.Begin(camera, lightTex,
                      AIForge::SpriteBatcher::BlendMode::Additive);
        for (auto& L : staticLights) {
            batcher.Submit(L.pos, {L.radius * 2, L.radius * 2}, L.color);
        }
        // 玩家火把(暖橙,摇曳)
        AIForge::Vec4 torch = {1.0f * flicker, 0.62f * flicker,
                               0.28f * flicker, 1.0f};
        batcher.Submit({charPos.x, charPos.y}, {620, 620}, torch);
        batcher.End();

        // ============ Pass 3 + 4: 合成 + Bloom -> 屏幕 ============
        post.Render(sceneFBO, lightFBO, lightingOn, bloomOn, winW, winH);

        // 标题栏
        titleAccum += dt;
        if (titleAccum >= 0.25f) {
            titleAccum = 0.0f;
            char buf[256];
            std::snprintf(buf, sizeof(buf),
                "AIForge Ch 06 | lighting=%s | bloom=%s | embers=%d | %.0f FPS",
                lightingOn ? "ON" : "OFF", bloomOn ? "ON" : "OFF",
                embers.GetAliveCount(), a.GetTime()->FPS());
            a.GetWindow()->SetTitle(buf);
        }
    });

    app->Run();
    app->Shutdown();
    std::printf("\n[Ch06] clean exit. Bye!\n");
    return 0;
}
