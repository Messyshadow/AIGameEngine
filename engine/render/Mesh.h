#pragma once

#include <cstdint>
#include <vector>

#include "../core/Math.h"

namespace AIForge {

/// @ai_summary 3D 网格:顶点(位置+法线+颜色)+ 索引,上传到 GPU 后可 Draw。
/// @ai_summary Ch 07 的核心数据类型。Ch 08 起会扩展加 UV / 切线。
/// @ai_example
///   Mesh cube;
///   MeshFactory::BuildCube(cube, 2.0f, {0.8f, 0.3f, 0.2f});
///   // 每帧:
///   shader.SetMat4("u_MVP", mvp);
///   cube.Draw();
/// @ai_related Camera3D, MeshFactory, Shader
class Mesh {
public:
    /// @ai_summary 单个顶点:位置 / 法线 / 颜色,各 3 个 float。
    struct Vertex {
        Vec3 pos;
        Vec3 normal;
        Vec3 color;
    };

    Mesh() = default;
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    /// @ai_summary 上传顶点 + 索引数据到 GPU(创建 VAO/VBO/EBO)。
    void SetData(const std::vector<Vertex>& vertices,
                 const std::vector<uint32_t>& indices);

    void Destroy();

    /// @ai_summary 发出一次 draw call(需先 Bind 好 shader)。
    void Draw() const;

    int  GetIndexCount()  const { return m_indexCount; }
    int  GetVertexCount() const { return m_vertexCount; }
    bool IsValid()        const { return m_vao != 0; }

private:
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    unsigned int m_ebo = 0;
    int          m_indexCount  = 0;
    int          m_vertexCount = 0;
};

/// @ai_summary 网格工厂:生成常见几何体到一个 Mesh 里。
namespace MeshFactory {

/// @ai_summary 生成一个以原点为中心、边长 size 的立方体(24 顶点,每面独立法线)。
void BuildCube(Mesh& out, float size, Vec3 color);

/// @ai_summary 生成一个 XZ 平面(地面),边长 size,法线朝 +Y。
void BuildPlane(Mesh& out, float size, Vec3 color);

/// @ai_summary 生成一个 UV 球体,半径 radius,经纬细分 segments。
void BuildSphere(Mesh& out, float radius, int segments, Vec3 color);

}  // namespace MeshFactory

}  // namespace AIForge
