// =============================================================================
// AIForge Engine — Chapter 08: PBR Materials + Lighting
// =============================================================================
// 本章 demo:经典的 PBR 测试球阵。
//
//   7x7 = 49 个球:
//     纵轴(下->上) metallic  0 -> 1
//     横轴(左->右) roughness 0.05 -> 1.0
//
//   光照:1 个方向光 + 4 个绕场景旋转的点光
//   着色:Cook-Torrance BRDF(GGX 分布 + Smith 几何 + Schlick 菲涅尔)
//
// 控制:
//   鼠标右键拖动  绕球阵旋转
//   滚轮          拉近/拉远
//   空格          暂停/继续点光旋转
//   ESC           退出
//
// 这一章的 Cook-Torrance 着色器是现代 3D 引擎(UE/Unity HDRP/Godot4)的画质基石。
// =============================================================================

#include <glad/gl.h>

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "engine/core/App.h"
#include "engine/render/Camera3D.h"
#include "engine/render/Light.h"
#include "engine/render/Material.h"
#include "engine/render/Mesh.h"
#include "engine/render/Shader.h"

// ---------------------------------------------------------------- shader ----
static const char* k_VS = R"GLSL(#version 450 core
layout(location=0) in vec3 a_Pos;
layout(location=1) in vec3 a_Normal;
layout(location=2) in vec3 a_Color;

uniform mat4 u_MVP;
uniform mat4 u_Model;

out vec3 v_WorldPos;
out vec3 v_Normal;

void main() {
    v_WorldPos  = vec3(u_Model * vec4(a_Pos, 1.0));
    v_Normal    = mat3(u_Model) * a_Normal;
    gl_Position = u_MVP * vec4(a_Pos, 1.0);
}
)GLSL";

static const char* k_FS = R"GLSL(#version 450 core
in vec3 v_WorldPos;
in vec3 v_Normal;
out vec4 frag;

uniform vec3  u_CamPos;
uniform vec3  u_Albedo;
uniform float u_Metallic;
uniform float u_Roughness;
uniform float u_AO;

uniform vec3  u_DirLightDir;
uniform vec3  u_DirLightColor;

#define MAX_PL 4
uniform int   u_NumPointLights;
uniform vec3  u_PointPos[MAX_PL];
uniform vec3  u_PointColor[MAX_PL];

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float rough) {
    float a  = rough * rough;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float d = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}
float GeometrySchlickGGX(float NdotV, float rough) {
    float r = rough + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float rough) {
    return GeometrySchlickGGX(max(dot(N, V), 0.0), rough)
         * GeometrySchlickGGX(max(dot(N, L), 0.0), rough);
}
vec3 FresnelSchlick(float cosT, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosT, 0.0, 1.0), 5.0);
}

vec3 ShadeOneLight(vec3 L, vec3 radiance, vec3 N, vec3 V, vec3 F0) {
    vec3 H = normalize(V + L);
    float NDF = DistributionGGX(N, H, u_Roughness);
    float G   = GeometrySmith(N, V, L, u_Roughness);
    vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denom;

    vec3 kD = (vec3(1.0) - F) * (1.0 - u_Metallic);
    float NdotL = max(dot(N, L), 0.0);
    return (kD * u_Albedo / PI + specular) * radiance * NdotL;
}

void main() {
    vec3 N  = normalize(v_Normal);
    vec3 V  = normalize(u_CamPos - v_WorldPos);
    vec3 F0 = mix(vec3(0.04), u_Albedo, u_Metallic);

    vec3 Lo = vec3(0.0);

    // 方向光
    Lo += ShadeOneLight(normalize(-u_DirLightDir), u_DirLightColor, N, V, F0);

    // 点光(距离平方反比衰减)
    for (int i = 0; i < u_NumPointLights; ++i) {
        vec3  d    = u_PointPos[i] - v_WorldPos;
        float dist = length(d);
        vec3  L    = d / dist;
        vec3  radiance = u_PointColor[i] / (dist * dist);
        Lo += ShadeOneLight(L, radiance, N, V, F0);
    }

    vec3 ambient = vec3(0.03) * u_Albedo * u_AO;
    vec3 color   = ambient + Lo;

    color = color / (color + vec3(1.0));   // Reinhard tonemap
    color = pow(color, vec3(1.0 / 2.2));   // gamma
    frag  = vec4(color, 1.0);
}
)GLSL";

int main(int /*argc*/, char* /*argv*/[]) {
    auto app = AIForge::App::Create();
    if (!app) return 1;

    AIForge::Window::Config wc;
    wc.title  = "AIForge Ch 08 - PBR Materials + Lighting";
    wc.width  = 1280;
    wc.height = 720;
    wc.vsync  = true;
    app->SetWindowConfig(wc);
    if (!app->Init()) return 1;
    app->SetShowFPSInTitle(false);
    app->SetClearColor(0.02f, 0.02f, 0.03f, 1.0f);

    glEnable(GL_DEPTH_TEST);

    // ---- shader ----
    AIForge::Shader shader;
    if (!shader.LoadFromSource(k_VS, k_FS)) {
        std::fprintf(stderr, "[Ch08] PBR shader compile failed\n");
        return 1;
    }

    // ---- 一个球网格,复用 49 次 ----
    AIForge::Mesh sphere;
    AIForge::MeshFactory::BuildSphere(sphere, 1.0f, 48, {1, 1, 1});

    // ---- 摄像机 ----
    AIForge::Camera3D cam;
    cam.mode        = AIForge::Camera3D::Mode::Orbit;
    cam.target      = {0.0f, 0.0f, 0.0f};
    cam.distance    = 26.0f;
    cam.orbitYaw    = 0.4f;
    cam.orbitPitch  = 0.25f;

    // ---- 光照 ----
    AIForge::DirectionalLight sun;
    sun.direction = {-0.4f, -0.7f, -0.5f};
    sun.color     = {1.0f, 0.97f, 0.92f};
    sun.intensity = 2.2f;

    AIForge::Vec3 pointColors[4] = {
        {1.00f, 0.55f, 0.20f},   // 暖橙
        {0.25f, 0.70f, 1.00f},   // 冷青
        {1.00f, 0.30f, 0.85f},   // 品红
        {0.45f, 1.00f, 0.45f},   // 绿
    };

    const int   GRID    = 7;
    const float SPACING = 2.6f;
    const float albedoR = 0.85f, albedoG = 0.32f, albedoB = 0.28f;  // 黏土红

    bool  lightsPaused = false;
    float lightTime    = 0.0f;
    float titleAccum   = 0.0f;

    std::printf("\n==================================================\n");
    std::printf(" AIForge Chapter 08 - PBR Materials + Lighting\n");
    std::printf("==================================================\n");
    std::printf(" 7x7 sphere grid:\n");
    std::printf("   vertical   : metallic  0 (bottom) -> 1 (top)\n");
    std::printf("   horizontal : roughness 0.05 (left) -> 1.0 (right)\n");
    std::printf(" RMB drag rotate | wheel zoom | SPACE pause lights | ESC quit\n");
    std::printf("==================================================\n\n");

    app->SetUpdateCallback([&](AIForge::App& a, float dt) {
        auto* in = a.GetInput();

        // ---- 输入 ----
        if (in->IsMouseDown(AIForge::MB_RIGHT)) {
            cam.OrbitDrag((float)in->MouseDX(), (float)in->MouseDY());
        }
        if (in->MouseWheel() != 0.0f) cam.Zoom(in->MouseWheel());
        if (in->IsKeyPressed(AIForge::K_SPACE)) lightsPaused = !lightsPaused;

        if (!lightsPaused) lightTime += dt;

        int winW = a.GetWindow()->GetWidth();
        int winH = a.GetWindow()->GetHeight();
        float aspect = (float)winW / (float)winH;

        AIForge::Mat4 view = cam.GetView();
        AIForge::Mat4 proj = cam.GetProjection(aspect);
        AIForge::Vec3 eye  = cam.GetEyePosition();

        // ---- 4 个点光绕场景旋转 ----
        AIForge::Vec3 pPos[4];
        for (int i = 0; i < 4; ++i) {
            float ang = lightTime * 0.6f + (float)i * 1.5708f;
            pPos[i] = {std::cos(ang) * 16.0f, 6.0f + 3.0f * std::sin(ang * 1.3f),
                       std::sin(ang) * 16.0f};
        }

        // ---- 公共 uniform ----
        shader.Bind();
        shader.SetVec3("u_CamPos", eye);
        shader.SetVec3("u_Albedo", {albedoR, albedoG, albedoB});
        shader.SetFloat("u_AO", 1.0f);
        shader.SetVec3("u_DirLightDir", sun.direction);
        shader.SetVec3("u_DirLightColor",
                       {sun.color.x * sun.intensity, sun.color.y * sun.intensity,
                        sun.color.z * sun.intensity});
        shader.SetInt("u_NumPointLights", 4);
        for (int i = 0; i < 4; ++i) {
            char nameP[32], nameC[32];
            std::snprintf(nameP, sizeof(nameP), "u_PointPos[%d]", i);
            std::snprintf(nameC, sizeof(nameC), "u_PointColor[%d]", i);
            shader.SetVec3(nameP, pPos[i]);
            float I = 320.0f;
            shader.SetVec3(nameC, {pointColors[i].x * I, pointColors[i].y * I,
                                   pointColors[i].z * I});
        }

        // ---- 画 7x7 球阵 ----
        float half = (GRID - 1) * 0.5f * SPACING;
        for (int row = 0; row < GRID; ++row) {       // metallic
            for (int col = 0; col < GRID; ++col) {   // roughness
                float metallic  = (float)row / (GRID - 1);
                float roughness = std::clamp((float)col / (GRID - 1),
                                              0.05f, 1.0f);
                AIForge::Vec3 pos{col * SPACING - half,
                                  row * SPACING - half, 0.0f};
                AIForge::Mat4 model = AIForge::Mat4::Translate(pos);
                AIForge::Mat4 mvp   = proj * view * model;

                shader.SetMat4("u_MVP", mvp);
                shader.SetMat4("u_Model", model);
                shader.SetFloat("u_Metallic", metallic);
                shader.SetFloat("u_Roughness", roughness);
                sphere.Draw();
            }
        }

        // ---- 标题栏 ----
        titleAccum += dt;
        if (titleAccum >= 0.25f) {
            titleAccum = 0.0f;
            char buf[256];
            std::snprintf(buf, sizeof(buf),
                "AIForge Ch 08 - PBR | 49 spheres | lights=%s | %.0f FPS",
                lightsPaused ? "paused" : "rotating", a.GetTime()->FPS());
            a.GetWindow()->SetTitle(buf);
        }
    });

    app->Run();
    app->Shutdown();
    std::printf("\n[Ch08] clean exit. Bye!\n");
    return 0;
}
