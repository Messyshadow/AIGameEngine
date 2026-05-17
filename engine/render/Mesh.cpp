#include "Mesh.h"

#include <glad/gl.h>

#include <cmath>

namespace AIForge {

Mesh::~Mesh() { Destroy(); }

void Mesh::SetData(const std::vector<Vertex>& vertices,
                   const std::vector<uint32_t>& indices) {
    Destroy();
    if (vertices.empty() || indices.empty()) return;

    m_vertexCount = static_cast<int>(vertices.size());
    m_indexCount  = static_cast<int>(indices.size());

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(vertices.size() * sizeof(Vertex)),
                 vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 (GLsizeiptr)(indices.size() * sizeof(uint32_t)),
                 indices.data(), GL_STATIC_DRAW);

    // 顶点格式:pos(3) + normal(3) + color(3) = 9 floats
    constexpr GLsizei stride = sizeof(Vertex);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Mesh::Destroy() {
    if (m_ebo) glDeleteBuffers(1, &m_ebo), m_ebo = 0;
    if (m_vbo) glDeleteBuffers(1, &m_vbo), m_vbo = 0;
    if (m_vao) glDeleteVertexArrays(1, &m_vao), m_vao = 0;
    m_indexCount = m_vertexCount = 0;
}

void Mesh::Draw() const {
    if (!m_vao) return;
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

// ====================================================================
//  MeshFactory
// ====================================================================
namespace MeshFactory {

void BuildCube(Mesh& out, float size, Vec3 color) {
    float h = size * 0.5f;
    std::vector<Mesh::Vertex> v;
    std::vector<uint32_t> idx;

    auto face = [&](Vec3 n, Vec3 a, Vec3 b, Vec3 c, Vec3 d) {
        uint32_t base = (uint32_t)v.size();
        v.push_back({a, n, color});
        v.push_back({b, n, color});
        v.push_back({c, n, color});
        v.push_back({d, n, color});
        idx.insert(idx.end(),
                   {base, base + 1, base + 2, base + 2, base + 3, base});
    };

    face({ 1, 0, 0}, { h,-h, h}, { h,-h,-h}, { h, h,-h}, { h, h, h});  // +X
    face({-1, 0, 0}, {-h,-h,-h}, {-h,-h, h}, {-h, h, h}, {-h, h,-h});  // -X
    face({ 0, 1, 0}, {-h, h, h}, { h, h, h}, { h, h,-h}, {-h, h,-h});  // +Y
    face({ 0,-1, 0}, {-h,-h,-h}, { h,-h,-h}, { h,-h, h}, {-h,-h, h});  // -Y
    face({ 0, 0, 1}, {-h,-h, h}, { h,-h, h}, { h, h, h}, {-h, h, h});  // +Z
    face({ 0, 0,-1}, { h,-h,-h}, {-h,-h,-h}, {-h, h,-h}, { h, h,-h});  // -Z

    out.SetData(v, idx);
}

void BuildPlane(Mesh& out, float size, Vec3 color) {
    float h = size * 0.5f;
    Vec3 n{0, 1, 0};
    std::vector<Mesh::Vertex> v = {
        {{-h, 0, -h}, n, color},
        {{ h, 0, -h}, n, color},
        {{ h, 0,  h}, n, color},
        {{-h, 0,  h}, n, color},
    };
    std::vector<uint32_t> idx = {0, 1, 2, 2, 3, 0};
    out.SetData(v, idx);
}

void BuildSphere(Mesh& out, float radius, int segments, Vec3 color) {
    if (segments < 3) segments = 3;
    const float PI = 3.14159265358979f;
    std::vector<Mesh::Vertex> v;
    std::vector<uint32_t> idx;

    // 经纬细分:rings (纬度) × segments (经度)
    int rings = segments;
    for (int y = 0; y <= rings; ++y) {
        float vY  = (float)y / rings;        // 0..1
        float lat = vY * PI;                 // 0..pi
        for (int x = 0; x <= segments; ++x) {
            float vX  = (float)x / segments; // 0..1
            float lon = vX * 2.0f * PI;      // 0..2pi
            Vec3 n{std::sin(lat) * std::cos(lon),
                   std::cos(lat),
                   std::sin(lat) * std::sin(lon)};
            v.push_back({n * radius, n, color});
        }
    }
    int stride = segments + 1;
    for (int y = 0; y < rings; ++y) {
        for (int x = 0; x < segments; ++x) {
            uint32_t i0 = y * stride + x;
            uint32_t i1 = i0 + 1;
            uint32_t i2 = i0 + stride;
            uint32_t i3 = i2 + 1;
            idx.insert(idx.end(), {i0, i2, i1, i1, i2, i3});
        }
    }
    out.SetData(v, idx);
}

}  // namespace MeshFactory

}  // namespace AIForge
