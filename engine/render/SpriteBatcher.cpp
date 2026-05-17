#include "SpriteBatcher.h"

#include <glad/gl.h>

#include <cmath>
#include <cstdio>
#include <vector>

#include "Camera2D.h"
#include "Shader.h"
#include "Texture.h"

namespace AIForge {

namespace {

// 内置 sprite shader:位置 + UV + 颜色,纹理相乘
const char* k_VertSrc = R"GLSL(#version 450 core
layout(location=0) in vec2 a_Pos;
layout(location=1) in vec2 a_UV;
layout(location=2) in vec4 a_Color;

uniform mat4 u_ViewProj;

out vec2 v_UV;
out vec4 v_Color;

void main() {
    gl_Position = u_ViewProj * vec4(a_Pos, 0.0, 1.0);
    v_UV = a_UV;
    v_Color = a_Color;
}
)GLSL";

const char* k_FragSrc = R"GLSL(#version 450 core
in vec2 v_UV;
in vec4 v_Color;
out vec4 frag;

uniform sampler2D u_Texture;

void main() {
    vec4 t = texture(u_Texture, v_UV);
    frag = t * v_Color;
    if (frag.a < 0.01) discard;
}
)GLSL";

}  // namespace

SpriteBatcher::SpriteBatcher() = default;
SpriteBatcher::~SpriteBatcher() { Shutdown(); }

bool SpriteBatcher::Init(int maxSprites) {
    if (m_vao != 0) return true;  // 已初始化

    m_maxSprites = maxSprites > 0 ? maxSprites : 1024;
    m_cpuBuffer.reserve(m_maxSprites * 4);

    // 1) Shader
    m_shader = new Shader();
    if (!m_shader->LoadFromSource(k_VertSrc, k_FragSrc)) {
        std::fprintf(stderr, "[SpriteBatcher] shader compile failed\n");
        delete m_shader;
        m_shader = nullptr;
        return false;
    }

    // 2) VAO
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // 3) VBO — 预分配空间(动态绘制)
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(m_maxSprites * 4 * sizeof(Vertex)),
                 nullptr, GL_DYNAMIC_DRAW);

    // 顶点格式: pos(2) + uv(2) + color(4) = 8 floats / 顶点
    constexpr GLsizei stride = sizeof(Vertex);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(Vertex, x));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(Vertex, u));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(Vertex, r));
    glEnableVertexAttribArray(2);

    // 4) EBO — 预填好所有 quad 的索引(永不变):每个 sprite 6 索引,4 顶点
    std::vector<unsigned int> indices;
    indices.reserve(m_maxSprites * 6);
    for (int i = 0; i < m_maxSprites; ++i) {
        unsigned int base = i * 4;
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
        indices.push_back(base + 0);
    }
    glGenBuffers(1, &m_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 (GLsizeiptr)(indices.size() * sizeof(unsigned int)),
                 indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // 启用 alpha 混合(让透明边缘自然)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::printf("[SpriteBatcher] initialized (maxSprites=%d)\n", m_maxSprites);
    return true;
}

void SpriteBatcher::Shutdown() {
    if (m_vbo) glDeleteBuffers(1, &m_vbo), m_vbo = 0;
    if (m_ebo) glDeleteBuffers(1, &m_ebo), m_ebo = 0;
    if (m_vao) glDeleteVertexArrays(1, &m_vao), m_vao = 0;
    if (m_shader) {
        delete m_shader;
        m_shader = nullptr;
    }
    m_cpuBuffer.clear();
    m_currentTex = nullptr;
}

void SpriteBatcher::Begin(const Camera2D& cam, const Texture& tex,
                          BlendMode blend) {
    m_viewProj   = cam.GetViewProjection();
    m_currentTex = &tex;
    m_blendMode  = blend;
    m_cpuBuffer.clear();
    m_spritesInBatch = 0;
    m_drawCalls = 0;
    m_spritesSubmitted = 0;
}

void SpriteBatcher::Submit(Vec2 pos, Vec2 size, Vec4 color, float rotation) {
    SubmitUV(pos, size, color, rotation, Vec4{0.0f, 0.0f, 1.0f, 1.0f});
}

void SpriteBatcher::SubmitUV(Vec2 pos, Vec2 size, Vec4 color, float rotation,
                              const Vec4& uvRect) {
    if (m_spritesInBatch >= m_maxSprites) {
        Flush();
    }

    float hw = size.x * 0.5f;
    float hh = size.y * 0.5f;

    // 本地空间 4 个角 (左下/右下/右上/左上)
    float lx[4] = {-hw,  hw,  hw, -hw};
    float ly[4] = {-hh, -hh,  hh,  hh};
    // UV 矩形: x=u0 y=v0 z=u1 w=v1
    float u[4]  = { uvRect.x, uvRect.z, uvRect.z, uvRect.x};
    float v[4]  = { uvRect.y, uvRect.y, uvRect.w, uvRect.w};

    float c = std::cos(rotation);
    float s = std::sin(rotation);

    for (int i = 0; i < 4; ++i) {
        Vertex vert;
        vert.x = lx[i] * c - ly[i] * s + pos.x;
        vert.y = lx[i] * s + ly[i] * c + pos.y;
        vert.u = u[i];
        vert.v = v[i];
        vert.r = color.x;
        vert.g = color.y;
        vert.b = color.z;
        vert.a = color.w;
        m_cpuBuffer.push_back(vert);
    }
    ++m_spritesInBatch;
    ++m_spritesSubmitted;
}

void SpriteBatcher::End() {
    if (m_spritesInBatch > 0) Flush();
}

void SpriteBatcher::Flush() {
    if (m_spritesInBatch == 0 || !m_shader || !m_currentTex) return;

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    (GLsizeiptr)(m_cpuBuffer.size() * sizeof(Vertex)),
                    m_cpuBuffer.data());

    // 根据混合模式设置 blend func
    if (m_blendMode == BlendMode::Additive) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);            // 加法:适合光源/发光
    } else {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // 普通透明
    }

    m_shader->Bind();
    m_shader->SetMat4("u_ViewProj", m_viewProj);
    m_shader->SetInt("u_Texture", 0);
    m_currentTex->Bind(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glDrawElements(GL_TRIANGLES, m_spritesInBatch * 6, GL_UNSIGNED_INT,
                   nullptr);

    glBindVertexArray(0);
    ++m_drawCalls;

    // 清空当前批次缓冲,准备下一批(若超过 maxSprites)
    m_cpuBuffer.clear();
    m_spritesInBatch = 0;
}

}  // namespace AIForge
