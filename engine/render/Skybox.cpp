#include "Skybox.h"

#include <glad/gl.h>

#include "Texture.h"

namespace AIForge {

namespace {

const char* k_VS = R"GLSL(#version 450 core
layout(location=0) in vec3 a_Pos;
layout(location=1) in vec3 a_Normal;  // unused
layout(location=2) in vec3 a_Color;   // unused
uniform mat4 u_ViewProjNoTrans;
out vec3 v_Dir;
void main() {
    v_Dir = a_Pos;
    vec4 p = u_ViewProjNoTrans * vec4(a_Pos, 1.0);
    gl_Position = p.xyww;   // z=w -> 透视除法后深度=1(远平面)
}
)GLSL";

const char* k_FS = R"GLSL(#version 450 core
in vec3 v_Dir;
out vec4 frag;
uniform sampler2D u_Equirect;

const vec2 invAtan = vec2(0.15915494, 0.31830989);  // (1/2pi, 1/pi)
vec2 dirToUV(vec3 d) {
    vec2 uv = vec2(atan(d.z, d.x), asin(clamp(d.y, -1.0, 1.0)));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() {
    vec3 d = normalize(v_Dir);
    vec3 c = texture(u_Equirect, dirToUV(d)).rgb;
    c = c / (c + vec3(1.0));        // Reinhard tonemap
    c = pow(c, vec3(1.0 / 2.2));    // gamma
    frag = vec4(c, 1.0);
}
)GLSL";

}  // namespace

bool Skybox::Init() {
    MeshFactory::BuildCube(m_cube, 2.0f, {1, 1, 1});
    if (!m_shader.LoadFromSource(k_VS, k_FS)) return false;
    return true;
}

void Skybox::Shutdown() {
    m_cube.Destroy();
    m_shader.Destroy();
}

void Skybox::Render(const Texture& env, const Mat4& view, const Mat4& proj) {
    // 去掉视图矩阵的平移 -> 天空盒永远以相机为中心
    Mat4 viewNoTrans = view;
    viewNoTrans.m[12] = 0.0f;
    viewNoTrans.m[13] = 0.0f;
    viewNoTrans.m[14] = 0.0f;
    Mat4 vp = proj * viewNoTrans;

    glDepthFunc(GL_LEQUAL);  // 天空盒深度=1,LEQUAL 才能在背景处通过
    m_shader.Bind();
    m_shader.SetMat4("u_ViewProjNoTrans", vp);
    env.Bind(0);
    m_shader.SetInt("u_Equirect", 0);
    m_cube.Draw();
    glDepthFunc(GL_LESS);    // 恢复默认
}

}  // namespace AIForge
