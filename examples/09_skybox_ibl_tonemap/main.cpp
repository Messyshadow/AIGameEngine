// =============================================================================
// AIForge Engine — Chapter 09: Skybox + IBL + Tonemap
// =============================================================================
// 本章 demo:Ch 08 的 PBR 球阵,现在沐浴在真实 HDR 环境里。
//
//   - 背景:HDR 天空盒(equirect 直采)
//   - 光照:1 个方向光 + IBL(基于图像的环境光照)
//   - 金属球反射天空;粗糙度越大反射越模糊
//   - 数字键 1/2/3 切换 3 张 Polyhaven CC0 HDRI
//
// 对比 Ch 08:右上角粗糙金属球不再发黑 —— 它现在反射环境了。
//
// 控制:RMB 拖动旋转 | 滚轮缩放 | 1/2/3 切环境 | ESC 退出
// =============================================================================

#include <glad/gl.h>

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "engine/core/App.h"
#include "engine/render/Camera3D.h"
#include "engine/render/Mesh.h"
#include "engine/render/Shader.h"
#include "engine/render/Skybox.h"
#include "engine/render/Texture.h"

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

uniform sampler2D u_Env;
uniform float     u_EnvMaxMip;

const float PI = 3.14159265359;
const vec2 invAtan = vec2(0.15915494, 0.31830989);
vec2 dirToUV(vec3 d) {
    vec2 uv = vec2(atan(d.z, d.x), asin(clamp(d.y, -1.0, 1.0)));
    uv *= invAtan; uv += 0.5;
    return uv;
}

float DistributionGGX(vec3 N, vec3 H, float r) {
    float a = r*r; float a2 = a*a;
    float nh = max(dot(N,H),0.0);
    float d = nh*nh*(a2-1.0)+1.0;
    return a2/(PI*d*d);
}
float GeometrySchlickGGX(float nv, float r) {
    float k = (r+1.0)*(r+1.0)/8.0;
    return nv/(nv*(1.0-k)+k);
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float r) {
    return GeometrySchlickGGX(max(dot(N,V),0.0),r)
         * GeometrySchlickGGX(max(dot(N,L),0.0),r);
}
vec3 FresnelSchlick(float c, vec3 F0) {
    return F0 + (1.0-F0)*pow(clamp(1.0-c,0.0,1.0),5.0);
}
vec3 FresnelSchlickRough(float c, vec3 F0, float r) {
    return F0 + (max(vec3(1.0-r),F0)-F0)*pow(clamp(1.0-c,0.0,1.0),5.0);
}

void main() {
    vec3 N = normalize(v_Normal);
    vec3 V = normalize(u_CamPos - v_WorldPos);
    vec3 R = reflect(-V, N);
    vec3 F0 = mix(vec3(0.04), u_Albedo, u_Metallic);

    // ---- 直接光:方向光 ----
    vec3 Lo = vec3(0.0);
    {
        vec3 L = normalize(-u_DirLightDir);
        vec3 H = normalize(V+L);
        float NDF = DistributionGGX(N,H,u_Roughness);
        float G   = GeometrySmith(N,V,L,u_Roughness);
        vec3  F   = FresnelSchlick(max(dot(H,V),0.0), F0);
        vec3 spec = NDF*G*F / (4.0*max(dot(N,V),0.0)*max(dot(N,L),0.0)+0.0001);
        vec3 kD = (vec3(1.0)-F)*(1.0-u_Metallic);
        Lo += (kD*u_Albedo/PI + spec) * u_DirLightColor * max(dot(N,L),0.0);
    }

    // ---- IBL 环境光 ----
    vec3 F  = FresnelSchlickRough(max(dot(N,V),0.0), F0, u_Roughness);
    vec3 kD = (vec3(1.0)-F)*(1.0-u_Metallic);
    vec3 irradiance  = textureLod(u_Env, dirToUV(N), max(u_EnvMaxMip-2.0,0.0)).rgb;
    vec3 diffuseIBL  = irradiance * u_Albedo;
    vec3 prefiltered = textureLod(u_Env, dirToUV(R), u_Roughness * u_EnvMaxMip).rgb;
    vec3 specularIBL = prefiltered * F;
    vec3 ambient = (kD * diffuseIBL + specularIBL) * u_AO;

    vec3 color = ambient + Lo;
    color = color / (color + vec3(1.0));   // Reinhard tonemap
    color = pow(color, vec3(1.0/2.2));     // gamma
    frag  = vec4(color, 1.0);
}
)GLSL";

int main(int /*argc*/, char* /*argv*/[]) {
    auto app = AIForge::App::Create();
    if (!app) return 1;

    AIForge::Window::Config wc;
    wc.title  = "AIForge Ch 09 - Skybox + IBL";
    wc.width  = 1280;
    wc.height = 720;
    wc.vsync  = true;
    app->SetWindowConfig(wc);
    if (!app->Init()) return 1;
    app->SetShowFPSInTitle(false);

    glEnable(GL_DEPTH_TEST);

    // ---- 3 张 HDRI ----
    const char* hdrPaths[3] = {
        "data/textures/hdri/venice_sunset_1k.hdr",
        "data/textures/hdri/kloppenheim_06_1k.hdr",
        "data/textures/hdri/studio_small_08_1k.hdr",
    };
    const char* hdrNames[3] = {"venice_sunset", "kloppenheim_06", "studio_small_08"};
    AIForge::Texture envMaps[3];
    int loadedCount = 0;
    for (int i = 0; i < 3; ++i) {
        if (envMaps[i].CreateFromHDR(hdrPaths[i])) ++loadedCount;
    }
    if (loadedCount == 0) {
        std::fprintf(stderr, "[Ch09] FATAL: no HDRI loaded — check data/textures/hdri/\n");
        return 1;
    }
    int activeEnv = 0;

    // ---- 天空盒 ----
    AIForge::Skybox skybox;
    if (!skybox.Init()) { std::fprintf(stderr, "[Ch09] skybox init failed\n"); return 1; }

    // ---- PBR+IBL shader ----
    AIForge::Shader shader;
    if (!shader.LoadFromSource(k_VS, k_FS)) {
        std::fprintf(stderr, "[Ch09] PBR+IBL shader compile failed\n");
        return 1;
    }

    // ---- 球阵 ----
    AIForge::Mesh sphere;
    AIForge::MeshFactory::BuildSphere(sphere, 1.0f, 48, {1, 1, 1});

    AIForge::Camera3D cam;
    cam.mode       = AIForge::Camera3D::Mode::Orbit;
    cam.target     = {0, 0, 0};
    cam.distance   = 26.0f;
    cam.orbitYaw   = 0.4f;
    cam.orbitPitch = 0.15f;

    const int   GRID    = 7;
    const float SPACING = 2.6f;
    AIForge::Vec3 albedo{0.92f, 0.92f, 0.95f};   // 近银白:让金属忠实反射环境

    float titleAccum = 0.0f;

    std::printf("\n==================================================\n");
    std::printf(" AIForge Chapter 09 - Skybox + IBL\n");
    std::printf("==================================================\n");
    std::printf(" 7x7 PBR spheres in a real HDR environment.\n");
    std::printf(" RMB drag rotate | wheel zoom | 1/2/3 switch HDRI | ESC quit\n");
    std::printf(" Loaded %d/3 HDRIs.\n", loadedCount);
    std::printf("==================================================\n\n");

    app->SetUpdateCallback([&](AIForge::App& a, float dt) {
        auto* in = a.GetInput();

        if (in->IsMouseDown(AIForge::MB_RIGHT))
            cam.OrbitDrag((float)in->MouseDX(), (float)in->MouseDY());
        if (in->MouseWheel() != 0.0f) cam.Zoom(in->MouseWheel());
        if (in->IsKeyPressed(AIForge::K_1)) activeEnv = 0;
        if (in->IsKeyPressed(AIForge::K_2)) activeEnv = 1;
        if (in->IsKeyPressed(AIForge::K_3)) activeEnv = 2;
        if (!envMaps[activeEnv].IsValid()) activeEnv = 0;

        int winW = a.GetWindow()->GetWidth();
        int winH = a.GetWindow()->GetHeight();
        float aspect = (float)winW / (float)winH;

        AIForge::Mat4 view = cam.GetView();
        AIForge::Mat4 proj = cam.GetProjection(aspect);
        AIForge::Vec3 eye  = cam.GetEyePosition();
        AIForge::Texture& env = envMaps[activeEnv];

        // ---- 画 7x7 球阵(PBR + IBL)----
        shader.Bind();
        shader.SetVec3("u_CamPos", eye);
        shader.SetVec3("u_Albedo", albedo);
        shader.SetFloat("u_AO", 1.0f);
        shader.SetVec3("u_DirLightDir", {-0.5f, -0.7f, -0.4f});
        shader.SetVec3("u_DirLightColor", {1.6f, 1.55f, 1.45f});
        env.Bind(0);
        shader.SetInt("u_Env", 0);
        shader.SetFloat("u_EnvMaxMip", (float)env.GetMaxMipLevel());

        float half = (GRID - 1) * 0.5f * SPACING;
        for (int row = 0; row < GRID; ++row) {
            for (int col = 0; col < GRID; ++col) {
                float metallic  = (float)row / (GRID - 1);
                float roughness = std::clamp((float)col / (GRID - 1), 0.05f, 1.0f);
                AIForge::Vec3 pos{col * SPACING - half, row * SPACING - half, 0.0f};
                AIForge::Mat4 model = AIForge::Mat4::Translate(pos);
                AIForge::Mat4 mvp   = proj * view * model;
                shader.SetMat4("u_MVP", mvp);
                shader.SetMat4("u_Model", model);
                shader.SetFloat("u_Metallic", metallic);
                shader.SetFloat("u_Roughness", roughness);
                sphere.Draw();
            }
        }

        // ---- 画天空盒(在物体之后,深度=远平面)----
        skybox.Render(env, view, proj);

        // ---- 标题 ----
        titleAccum += dt;
        if (titleAccum >= 0.25f) {
            titleAccum = 0.0f;
            char buf[256];
            std::snprintf(buf, sizeof(buf),
                "AIForge Ch 09 - IBL | env=%s [%d/3] | %.0f FPS",
                hdrNames[activeEnv], activeEnv + 1, a.GetTime()->FPS());
            a.GetWindow()->SetTitle(buf);
        }
    });

    app->Run();
    app->Shutdown();
    std::printf("\n[Ch09] clean exit. Bye!\n");
    return 0;
}
