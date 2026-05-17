// =============================================================================
// AIForge Engine — Chapter 05: 2D Animation + Particles + Tilemap
// =============================================================================
// 本章 demo:一个机甲角色,在程序化生成的瓦片地图上走动,自带尘土拖尾 + 闪光爆炸。
//
// 三个系统一次性串起来:
//   - Tilemap        程序化 32x24 网格(草地/沙滩/水/石头)
//   - SpriteAnimation idle(6帧) <-> walk(8帧) 状态切换 + 水平翻转
//   - ParticleSystem  脚下尘土 + 空格爆破闪光
//
// 渲染流程(3 个 draw call,因为有 3 张不同的纹理):
//   1) Tilemap   bind white_pixel -> 上色 -> draw
//   2) Particles bind soft_circle -> draw
//   3) Character bind robot3_*    -> draw (UV 子矩形从 sheet 取)
//
// 控制:
//   WASD   移动(松开 -> idle,移动 -> walk;水平翻转跟随朝向)
//   SPACE  在角色位置爆 80 个金色火花粒子
//   R      角色回中
//   ESC    退出
// =============================================================================

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <vector>

#include "engine/core/App.h"
#include "engine/render/Camera2D.h"
#include "engine/render/ParticleSystem.h"
#include "engine/render/SpriteAnimation.h"
#include "engine/render/SpriteBatcher.h"
#include "engine/render/SpriteSheet.h"
#include "engine/render/Texture.h"
#include "engine/render/Tilemap.h"

static std::vector<uint8_t> MakeSoftCircleRGBA(int size) {
    std::vector<uint8_t> px((size_t)size * size * 4, 0);
    float r = size * 0.5f;
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float dx = (x + 0.5f - r) / r;
            float dy = (y + 0.5f - r) / r;
            float d  = std::sqrt(dx * dx + dy * dy);
            float a = 1.0f - std::clamp((d - 0.6f) / 0.4f, 0.0f, 1.0f);
            size_t idx = ((size_t)y * size + x) * 4;
            px[idx + 0] = 255;
            px[idx + 1] = 255;
            px[idx + 2] = 255;
            px[idx + 3] = (uint8_t)(a * 255);
        }
    }
    return px;
}

int main(int /*argc*/, char* /*argv*/[]) {
    auto app = AIForge::App::Create();
    if (!app) return 1;

    AIForge::Window::Config wc;
    wc.title  = "AIForge Ch 05 - Animation + Particles + Tilemap";
    wc.width  = 1280;
    wc.height = 720;
    wc.vsync  = true;
    app->SetWindowConfig(wc);
    if (!app->Init()) return 1;

    app->SetClearColor(0.05f, 0.05f, 0.07f, 1.0f);
    app->SetShowFPSInTitle(false);

    // ----------------------------------------------- 加载素材 -----
    AIForge::Texture idleTex, walkTex;
    if (!idleTex.CreateFromFile("data/textures/character/robot3_idle.png",
                                 false /*mip*/, false /*nearest*/)) {
        std::fprintf(stderr,
            "[Ch05] FATAL: cannot load robot3_idle.png from "
            "'data/textures/character/'. CWD = '%s'?\n",
            "(check that build/bin/Release has data/ next to .exe)");
        return 1;
    }
    if (!walkTex.CreateFromFile("data/textures/character/robot3_walk.png",
                                 false, false)) {
        std::fprintf(stderr, "[Ch05] FATAL: cannot load robot3_walk.png\n");
        return 1;
    }

    // Sprite sheet 假设 200x200 cell。robot3_idle 是 1400x1400 -> 7 cols × 7 rows,
    // robot3_walk 是 1800x1000 -> 9 cols × 5 rows。如果视觉上裁切错误,
    // 在这两行改 cell 尺寸即可。
    AIForge::SpriteSheet idleSheet(idleTex, 200, 200);
    AIForge::SpriteSheet walkSheet(walkTex, 200, 200);
    std::printf("[Ch05] idle sheet: %d cells (%dx%d)\n", idleSheet.GetFrameCount(),
                idleSheet.GetCols(), idleSheet.GetRows());
    std::printf("[Ch05] walk sheet: %d cells (%dx%d)\n", walkSheet.GetFrameCount(),
                walkSheet.GetCols(), walkSheet.GetRows());

    // ----------------------------------------------- 动画 -----
    AIForge::SpriteAnimation idleAnim;
    idleAnim.frames = {0, 1, 2, 3, 4, 5};
    idleAnim.fps    = 6.0f;
    idleAnim.loop   = true;

    AIForge::SpriteAnimation walkAnim;
    walkAnim.frames = {0, 1, 2, 3, 4, 5, 6, 7};
    walkAnim.fps    = 12.0f;
    walkAnim.loop   = true;

    // ----------------------------------------------- Tilemap -----
    AIForge::Tilemap map;
    map.Init(32, 24, 64);
    map.GenerateProcedural();
    float mapW = (float)(32 * 64);
    float mapH = (float)(24 * 64);

    // ----------------------------------------------- 辅助纹理 -----
    AIForge::Texture whitePixel;
    uint8_t white[4] = {255, 255, 255, 255};
    whitePixel.CreateFromRGBA(1, 1, white);

    auto circlePixels = MakeSoftCircleRGBA(32);
    AIForge::Texture particleTex;
    particleTex.CreateFromRGBA(32, 32, circlePixels.data());

    // ----------------------------------------------- 渲染器 -----
    AIForge::SpriteBatcher batcher;
    if (!batcher.Init(10000)) return 1;

    AIForge::Camera2D camera;
    camera.SetViewport(1280, 720);
    camera.position = {mapW * 0.5f, mapH * 0.5f};

    // ----------------------------------------------- 粒子发射器 -----
    AIForge::ParticleEmitter dust;
    dust.rate         = 80.0f;
    dust.lifeMin      = 0.35f;
    dust.lifeMax      = 0.9f;
    dust.velAngleDeg  = 90.0f;     // 向上飘
    dust.velSpreadDeg = 60.0f;
    dust.velMagMin    = 30.0f;
    dust.velMagMax    = 110.0f;
    dust.gravity      = {0, -180.0f};
    dust.colorStart   = {0.95f, 0.85f, 0.55f, 0.85f};
    dust.colorEnd     = {0.55f, 0.40f, 0.18f, 0.0f};
    dust.sizeStart    = {12, 12};
    dust.sizeEnd      = {2, 2};
    dust.emitting     = false;
    dust.Init(2000);

    AIForge::ParticleEmitter sparks;
    sparks.rate         = 0.0f;      // 只 Burst
    sparks.lifeMin      = 0.5f;
    sparks.lifeMax      = 1.3f;
    sparks.velAngleDeg  = 90.0f;
    sparks.velSpreadDeg = 180.0f;    // 全圆爆发
    sparks.velMagMin    = 120.0f;
    sparks.velMagMax    = 420.0f;
    sparks.gravity      = {0, -120.0f};
    sparks.colorStart   = {1.0f, 1.0f, 0.40f, 1.0f};
    sparks.colorEnd     = {1.0f, 0.20f, 0.0f, 0.0f};
    sparks.sizeStart    = {14, 14};
    sparks.sizeEnd      = {2, 2};
    sparks.emitting     = false;
    sparks.Init(3000);

    // ----------------------------------------------- 角色状态 -----
    AIForge::Vec2 charPos    = {mapW * 0.5f, mapH * 0.5f};
    bool          isWalking  = false;
    bool          facingLeft = false;

    std::printf("\n");
    std::printf("==================================================\n");
    std::printf(" AIForge Chapter 05 - Animation + Particles + Tilemap\n");
    std::printf("==================================================\n");
    std::printf(" WASD     move character (idle <-> walk)\n");
    std::printf(" SPACE    burst 80 sparks at character\n");
    std::printf(" R        reset character to center\n");
    std::printf(" ESC      quit\n");
    std::printf("==================================================\n\n");

    float titleAccum = 0.0f;

    app->SetUpdateCallback([&](AIForge::App& a, float dt) {
        auto* in = a.GetInput();

        // 1) 角色移动
        AIForge::Vec2 dir{0, 0};
        if (in->IsKeyDown(AIForge::K_W)) dir.y += 1;
        if (in->IsKeyDown(AIForge::K_S)) dir.y -= 1;
        if (in->IsKeyDown(AIForge::K_A)) dir.x -= 1;
        if (in->IsKeyDown(AIForge::K_D)) dir.x += 1;

        isWalking = (dir.x != 0.0f || dir.y != 0.0f);
        if (dir.x < 0)      facingLeft = true;
        else if (dir.x > 0) facingLeft = false;

        if (isWalking) {
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            dir.x /= len; dir.y /= len;
            float speed = 250.0f;
            charPos.x += dir.x * speed * dt;
            charPos.y += dir.y * speed * dt;
        }
        charPos.x = std::clamp(charPos.x, 80.0f, mapW - 80.0f);
        charPos.y = std::clamp(charPos.y, 80.0f, mapH - 80.0f);

        if (in->IsKeyPressed(AIForge::K_R)) {
            charPos = {mapW * 0.5f, mapH * 0.5f};
        }

        // 2) 动画
        if (isWalking) walkAnim.Tick(dt);
        else           idleAnim.Tick(dt);

        // 3) 粒子
        dust.SetOrigin({charPos.x, charPos.y - 70.0f});
        dust.emitting = isWalking;
        dust.Update(dt);

        if (in->IsKeyPressed(AIForge::K_SPACE)) {
            sparks.SetOrigin(charPos);
            sparks.Burst(80);
        }
        sparks.Update(dt);

        // 4) 摄像机平滑跟随
        float follow = std::min(1.0f, 4.5f * dt);
        camera.position.x += (charPos.x - camera.position.x) * follow;
        camera.position.y += (charPos.y - camera.position.y) * follow;
        camera.SetViewport(a.GetWindow()->GetWidth(),
                           a.GetWindow()->GetHeight());

        // 5) 渲染三个 pass(每个 pass = 一个 draw call,因为换纹理)
        int totalDrawCalls = 0;

        // Pass 1: 瓦片地图
        batcher.Begin(camera, whitePixel);
        map.Render(batcher, camera);
        batcher.End();
        totalDrawCalls += batcher.GetDrawCallCount();

        // Pass 2: 所有粒子(尘土 + 火花)
        batcher.Begin(camera, particleTex);
        dust.Render(batcher);
        sparks.Render(batcher);
        batcher.End();
        totalDrawCalls += batcher.GetDrawCallCount();

        // Pass 3: 角色
        const AIForge::Texture*    curTex   = isWalking ? &walkTex  : &idleTex;
        const AIForge::SpriteSheet& curSheet = isWalking ? walkSheet : idleSheet;
        const AIForge::SpriteAnimation& curAnim =
            isWalking ? walkAnim : idleAnim;
        AIForge::Vec4 uv = curSheet.GetFrameUV(curAnim.GetCurrentFrameIndex());
        if (facingLeft) std::swap(uv.x, uv.z);  // 水平镜像 UV

        batcher.Begin(camera, *curTex);
        batcher.SubmitUV(charPos, {200, 200}, {1, 1, 1, 1}, 0.0f, uv);
        batcher.End();
        totalDrawCalls += batcher.GetDrawCallCount();

        // 6) 标题栏
        titleAccum += dt;
        if (titleAccum >= 0.2f) {
            titleAccum = 0.0f;
            char buf[256];
            std::snprintf(buf, sizeof(buf),
                "AIForge Ch 05 | %s | pos=(%.0f,%.0f) | particles=%d | "
                "draw_calls=%d | %.0f FPS",
                isWalking ? "walk" : "idle",
                charPos.x, charPos.y,
                dust.GetAliveCount() + sparks.GetAliveCount(),
                totalDrawCalls, a.GetTime()->FPS());
            a.GetWindow()->SetTitle(buf);
        }
    });

    app->Run();
    app->Shutdown();
    std::printf("\n[Ch05] clean exit. Bye!\n");
    return 0;
}
