#include "PostProcess.h"

#include <glad/gl.h>

#include <cstdio>

namespace AIForge {

namespace {

// 所有后处理 pass 共用的全屏顶点 shader
const char* k_FsVert = R"GLSL(#version 450 core
layout(location=0) in vec2 a_Pos;
layout(location=1) in vec2 a_UV;
out vec2 v_UV;
void main() {
    gl_Position = vec4(a_Pos, 0.0, 1.0);
    v_UV = a_UV;
}
)GLSL";

// 合成:场景色 × 光照(关灯则直接输出场景)
const char* k_CompositeFrag = R"GLSL(#version 450 core
in vec2 v_UV;
out vec4 frag;
uniform sampler2D u_Scene;
uniform sampler2D u_Light;
uniform int u_LightingEnabled;
void main() {
    vec3 scene = texture(u_Scene, v_UV).rgb;
    if (u_LightingEnabled == 1) {
        vec3 light = texture(u_Light, v_UV).rgb;
        frag = vec4(scene * light, 1.0);
    } else {
        frag = vec4(scene, 1.0);
    }
}
)GLSL";

// 提取高亮:亮度超过阈值的像素才保留
const char* k_BrightFrag = R"GLSL(#version 450 core
in vec2 v_UV;
out vec4 frag;
uniform sampler2D u_Tex;
uniform float u_Threshold;
void main() {
    vec3 c = texture(u_Tex, v_UV).rgb;
    float lum = dot(c, vec3(0.2126, 0.7152, 0.0722));
    frag = (lum > u_Threshold) ? vec4(c, 1.0) : vec4(0.0, 0.0, 0.0, 1.0);
}
)GLSL";

// 可分离高斯模糊(方向由 uniform 给)
const char* k_BlurFrag = R"GLSL(#version 450 core
in vec2 v_UV;
out vec4 frag;
uniform sampler2D u_Tex;
uniform vec2 u_Direction;   // (texelW,0)=水平  (0,texelH)=垂直
void main() {
    float w[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
    vec3 result = texture(u_Tex, v_UV).rgb * w[0];
    for (int i = 1; i < 5; ++i) {
        result += texture(u_Tex, v_UV + u_Direction * float(i)).rgb * w[i];
        result += texture(u_Tex, v_UV - u_Direction * float(i)).rgb * w[i];
    }
    frag = vec4(result, 1.0);
}
)GLSL";

// 最终合成:composite + bloom,再 Reinhard tonemap + gamma
const char* k_CombineFrag = R"GLSL(#version 450 core
in vec2 v_UV;
out vec4 frag;
uniform sampler2D u_Scene;
uniform sampler2D u_Bloom;
uniform int u_BloomEnabled;
void main() {
    vec3 c = texture(u_Scene, v_UV).rgb;
    if (u_BloomEnabled == 1) {
        c += texture(u_Bloom, v_UV).rgb;
    }
    c = c / (c + vec3(1.0));            // Reinhard tonemap
    c = pow(c, vec3(1.0 / 2.2));        // gamma
    frag = vec4(c, 1.0);
}
)GLSL";

}  // namespace

PostProcess::~PostProcess() { Shutdown(); }

bool PostProcess::Init(int width, int height) {
    m_width  = width  > 0 ? width  : 1;
    m_height = height > 0 ? height : 1;

    if (!m_compositeShader.LoadFromSource(k_FsVert, k_CompositeFrag)) return false;
    if (!m_brightShader.LoadFromSource(k_FsVert, k_BrightFrag))       return false;
    if (!m_blurShader.LoadFromSource(k_FsVert, k_BlurFrag))           return false;
    if (!m_combineShader.LoadFromSource(k_FsVert, k_CombineFrag))     return false;

    if (!m_compositeFBO.Create(m_width, m_height)) return false;
    if (!m_brightFBO.Create(m_width, m_height))    return false;
    if (!m_blurFBO[0].Create(m_width, m_height))   return false;
    if (!m_blurFBO[1].Create(m_width, m_height))   return false;

    // 全屏四边形:2 三角形,(x,y,u,v) 每顶点 4 float
    const float quad[] = {
        // pos      uv
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
    };
    glGenVertexArrays(1, &m_quadVAO);
    glBindVertexArray(m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    std::printf("[PostProcess] initialized (%dx%d)\n", m_width, m_height);
    return true;
}

void PostProcess::Resize(int width, int height) {
    if (width == m_width && height == m_height) return;
    m_width  = width  > 0 ? width  : 1;
    m_height = height > 0 ? height : 1;
    m_compositeFBO.Resize(m_width, m_height);
    m_brightFBO.Resize(m_width, m_height);
    m_blurFBO[0].Resize(m_width, m_height);
    m_blurFBO[1].Resize(m_width, m_height);
}

void PostProcess::Shutdown() {
    if (m_quadVBO) glDeleteBuffers(1, &m_quadVBO), m_quadVBO = 0;
    if (m_quadVAO) glDeleteVertexArrays(1, &m_quadVAO), m_quadVAO = 0;
}

void PostProcess::DrawFullscreenQuad() const {
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void PostProcess::Render(const Framebuffer& sceneFBO,
                         const Framebuffer& lightFBO, bool lightingEnabled,
                         bool bloomEnabled, int screenW, int screenH) {
    // 后处理 pass 不需要混合 / 深度
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    // ---- Pass 1: 合成 scene × light -> compositeFBO ----
    m_compositeFBO.Bind();
    m_compositeShader.Bind();
    sceneFBO.BindColorTexture(0);
    lightFBO.BindColorTexture(1);
    m_compositeShader.SetInt("u_Scene", 0);
    m_compositeShader.SetInt("u_Light", 1);
    m_compositeShader.SetInt("u_LightingEnabled", lightingEnabled ? 1 : 0);
    DrawFullscreenQuad();

    int lastBlur = 0;
    if (bloomEnabled) {
        // ---- Pass 2: 提取高亮 -> brightFBO ----
        m_brightFBO.Bind();
        m_brightShader.Bind();
        m_compositeFBO.BindColorTexture(0);
        m_brightShader.SetInt("u_Tex", 0);
        m_brightShader.SetFloat("u_Threshold", m_bloomThreshold);
        DrawFullscreenQuad();

        // ---- Pass 3: ping-pong 高斯模糊 ----
        float texelW = 1.0f / (float)m_width;
        float texelH = 1.0f / (float)m_height;
        bool horizontal = true;
        int passes = m_blurIterations * 2;
        for (int i = 0; i < passes; ++i) {
            int dst = horizontal ? 0 : 1;
            m_blurFBO[dst].Bind();
            m_blurShader.Bind();
            if (i == 0) {
                m_brightFBO.BindColorTexture(0);
            } else {
                m_blurFBO[horizontal ? 1 : 0].BindColorTexture(0);
            }
            m_blurShader.SetInt("u_Tex", 0);
            m_blurShader.SetVec2("u_Direction",
                                 horizontal ? Vec2{texelW, 0.0f}
                                            : Vec2{0.0f, texelH});
            DrawFullscreenQuad();
            lastBlur    = dst;
            horizontal  = !horizontal;
        }
    }

    // ---- Pass 4: 最终合成 -> 屏幕 ----
    Framebuffer::BindDefault(screenW, screenH);
    m_combineShader.Bind();
    m_compositeFBO.BindColorTexture(0);
    m_combineShader.SetInt("u_Scene", 0);
    if (bloomEnabled) {
        m_blurFBO[lastBlur].BindColorTexture(1);
        m_combineShader.SetInt("u_Bloom", 1);
        m_combineShader.SetInt("u_BloomEnabled", 1);
    } else {
        m_combineShader.SetInt("u_BloomEnabled", 0);
    }
    DrawFullscreenQuad();

    // 恢复混合状态(后续 SpriteBatcher 还要用)
    glEnable(GL_BLEND);
}

}  // namespace AIForge
