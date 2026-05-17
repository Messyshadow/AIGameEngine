// =============================================================================
// AIForge Engine — Chapter 07: 3D Mesh + Camera3D
// =============================================================================
// 本章 demo:AIForge 的第一个 3D 场景。
//   - 灰色地面
//   - 一圈 8 个旋转的彩色立方体
//   - 中心一个大球
//   - 一束方向光(立方体每个面亮度不同 -> 立体感)
//
// 控制:
//   鼠标右键拖动   Orbit 绕场景旋转 / FPS 模式下转视角
//   滚轮          拉近 / 拉远(Orbit)或改 FOV(FPS)
//   WASD          平移注视点(Orbit)/ 移动(FPS)
//   F             切换 Orbit / FPS 模式
//   ESC           退出
//
// 关键点:MVP 矩阵(gl_Position = Projection * View * Model * 顶点)。
// =============================================================================

#include <glad/gl.h>

#include <cmath>
#include <cstdio>
#include <vector>

#include "engine/core/App.h"
#include "engine/render/Camera3D.h"
#include "engine/render/Mesh.h"
#include "engine/render/Shader.h"

namespace {

const char* k_Vert = R"GLSL(#version 450 core
layout(location=0) in vec3 a_Pos;
layout(location=1) in vec3 a_Normal;
layout(location=2) in vec3 a_Color;

uniform mat4 u_MVP;
uniform mat4 u_Model;

out vec3 v_Normal;
out vec3 v_Color;

void main() {
    gl_Position = u_MVP * vec4(a_Pos, 1.0);
    v_Normal = mat3(u_Model) * a_Normal;   // 旋转+均匀缩放下足够准确
    v_Color  = a_Color;
}
)GLSL";

const char* k_Frag = R"GLSL(#version 450 core
in vec3 v_Normal;
in vec3 v_Color;
out vec4 frag;

uniform vec3 u_LightDir;   // 光"传播"方向
uniform vec3 u_Tint;

void main() {
    vec3 n       = normalize(v_Normal);
    vec3 toLight = normalize(-u_LightDir);
    float diff   = max(dot(n, toLight), 0.0);
    float ambient = 0.25;
    vec3 c = v_Color * u_Tint * (ambient + diff * 0.85);
    frag = vec4(c, 1.0);
}
)GLSL";

// 8 个立方体的彩虹色
const AIForge::Vec3 k_CubeColors[8] = {
    {1.00f, 0.30f, 0.30f}, {1.00f, 0.65f, 0.25f},
    {1.00f, 0.95f, 0.30f}, {0.40f, 0.90f, 0.35f},
    {0.30f, 0.85f, 0.85f}, {0.35f, 0.55f, 1.00f},
    {0.65f, 0.40f, 1.00f}, {1.00f, 0.40f, 0.85f},
};

}  // namespace

int main(int /*argc*/, char* /*argv*/[]) {
    auto app = AIForge::App::Create();
    if (!app) return 1;

    AIForge::Window::Config wc;
    wc.title  = "AIForge Ch 07 - 3D Mesh + Camera3D";
    wc.width  = 1280;
    wc.height = 720;
    wc.vsync  = true;
    app->SetWindowConfig(wc);
    if (!app->Init()) return 1;
    app->SetShowFPSInTitle(false);
    app->SetClearColor(0.06f, 0.07f, 0.09f, 1.0f);
    app->SetEscQuits(true);

    // 3D 必须开深度测试
    glEnable(GL_DEPTH_TEST);

    // ---- Shader ----
    AIForge::Shader shader;
    if (!shader.LoadFromSource(k_Vert, k_Frag)) {
        std::fprintf(stderr, "[Ch07] shader compile failed\n");
        return 1;
    }

    // ---- Meshes ----
    AIForge::Mesh cubeMesh, planeMesh, sphereMesh;
    AIForge::MeshFactory::BuildCube(cubeMesh, 1.6f, {1, 1, 1});
    AIForge::MeshFactory::BuildPlane(planeMesh, 40.0f, {1, 1, 1});
    AIForge::MeshFactory::BuildSphere(sphereMesh, 1.8f, 32, {1, 1, 1});

    // ---- Camera ----
    AIForge::Camera3D cam;
    cam.mode        = AIForge::Camera3D::Mode::Orbit;
    cam.target      = {0.0f, 0.5f, 0.0f};
    cam.distance    = 16.0f;
    cam.orbitYaw    = 0.7f;
    cam.orbitPitch  = 0.45f;

    std::printf("\n==================================================\n");
    std::printf(" AIForge Chapter 07 - 3D Mesh + Camera3D\n");
    std::printf("==================================================\n");
    std::printf(" Right-drag  orbit / look\n");
    std::printf(" Wheel       zoom\n");
    std::printf(" WASD        pan target / move\n");
    std::printf(" F           toggle Orbit / FPS\n");
    std::printf(" ESC         quit\n");
    std::printf("==================================================\n\n");

    AIForge::Vec3 lightDir = AIForge::Normalize({-0.4f, -0.8f, -0.45f});
    float titleAccum = 0.0f;

    app->SetUpdateCallback([&](AIForge::App& a, float dt) {
        auto* in = a.GetInput();
        int   winW = a.GetWindow()->GetWidth();
        int   winH = a.GetWindow()->GetHeight();
        float aspect = (float)winW / (float)(winH > 0 ? winH : 1);

        // ---- 输入 ----
        if (in->IsKeyPressed(AIForge::K_F)) {
            cam.mode = (cam.mode == AIForge::Camera3D::Mode::Orbit)
                           ? AIForge::Camera3D::Mode::FPS
                           : AIForge::Camera3D::Mode::Orbit;
        }

        // 右键拖动 = 旋转视角
        if (in->IsMouseDown(AIForge::MB_RIGHT)) {
            float dx = (float)in->MouseDX();
            float dy = (float)in->MouseDY();
            if (cam.mode == AIForge::Camera3D::Mode::Orbit)
                cam.OrbitDrag(dx, dy);
            else
                cam.FPSLook(dx, dy);
        }

        // 滚轮缩放
        if (in->MouseWheel() != 0.0f) cam.Zoom(in->MouseWheel());

        // WASD
        AIForge::Vec3 mv{0, 0, 0};
        if (in->IsKeyDown(AIForge::K_W)) mv.z += 1;
        if (in->IsKeyDown(AIForge::K_S)) mv.z -= 1;
        if (in->IsKeyDown(AIForge::K_D)) mv.x += 1;
        if (in->IsKeyDown(AIForge::K_A)) mv.x -= 1;
        if (in->IsKeyDown(AIForge::K_E)) mv.y += 1;
        if (in->IsKeyDown(AIForge::K_Q)) mv.y -= 1;

        if (cam.mode == AIForge::Camera3D::Mode::FPS) {
            cam.FPSMove(mv, dt, 8.0f);
        } else if (mv.x != 0 || mv.z != 0 || mv.y != 0) {
            // Orbit:平移注视点(沿摄像机朝向投影到地面)
            AIForge::Vec3 f = cam.GetForward();
            f.y = 0;
            f = AIForge::Normalize(f);
            AIForge::Vec3 right =
                AIForge::Normalize(AIForge::Cross(f, {0, 1, 0}));
            AIForge::Vec3 delta = right * mv.x + f * mv.z;
            delta.y = mv.y;
            cam.target = cam.target + delta * (8.0f * dt);
        }

        // ---- 矩阵 ----
        AIForge::Mat4 view = cam.GetView();
        AIForge::Mat4 proj = cam.GetProjection(aspect);
        AIForge::Mat4 vp   = proj * view;

        // ---- 渲染 ----
        shader.Bind();
        shader.SetVec3("u_LightDir", lightDir);

        float t = a.GetTime()->TotalTime();

        // 地面
        {
            AIForge::Mat4 model = AIForge::Mat4::Translate({0, -1.5f, 0});
            shader.SetMat4("u_Model", model);
            shader.SetMat4("u_MVP", vp * model);
            shader.SetVec3("u_Tint", {0.35f, 0.37f, 0.40f});
            planeMesh.Draw();
        }

        // 8 个旋转立方体,排成一圈
        const float PI = 3.14159265f;
        for (int i = 0; i < 8; ++i) {
            float ang = (float)i / 8.0f * 2.0f * PI;
            float radius = 6.0f;
            AIForge::Vec3 pos{std::cos(ang) * radius, 0.4f,
                              std::sin(ang) * radius};
            AIForge::Mat4 model =
                AIForge::Mat4::Translate(pos) *
                AIForge::Mat4::RotateY(t * 1.2f + (float)i) *
                AIForge::Mat4::RotateX(t * 0.7f);
            shader.SetMat4("u_Model", model);
            shader.SetMat4("u_MVP", vp * model);
            shader.SetVec3("u_Tint", k_CubeColors[i]);
            cubeMesh.Draw();
        }

        // 中心球
        {
            AIForge::Mat4 model =
                AIForge::Mat4::Translate({0, 0.6f, 0}) *
                AIForge::Mat4::RotateY(t * 0.4f);
            shader.SetMat4("u_Model", model);
            shader.SetMat4("u_MVP", vp * model);
            shader.SetVec3("u_Tint", {0.95f, 0.85f, 0.55f});
            sphereMesh.Draw();
        }

        // ---- 标题栏 ----
        titleAccum += dt;
        if (titleAccum >= 0.25f) {
            titleAccum = 0.0f;
            AIForge::Vec3 eye = cam.GetEyePosition();
            char buf[256];
            std::snprintf(buf, sizeof(buf),
                "AIForge Ch 07 | mode=%s | eye=(%.1f,%.1f,%.1f) | %.0f FPS",
                cam.mode == AIForge::Camera3D::Mode::Orbit ? "Orbit" : "FPS",
                eye.x, eye.y, eye.z, a.GetTime()->FPS());
            a.GetWindow()->SetTitle(buf);
        }
    });

    app->Run();
    app->Shutdown();
    std::printf("\n[Ch07] clean exit. Bye!\n");
    return 0;
}
