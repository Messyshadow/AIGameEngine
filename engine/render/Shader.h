#pragma once

#include <string>
#include <unordered_map>

#include "../core/Math.h"

namespace AIForge {

/// @ai_summary OpenGL GLSL Shader 封装 — 编译 vs/fs + 链接 + 设置 uniform。
/// @ai_summary 内部缓存 uniform location,反复 SetUniform 不会卡。
/// @ai_example
///   Shader sh;
///   if (!sh.LoadFromSource(vsrc, fsrc)) { ... handle error ... }
///   sh.Bind();
///   sh.SetMat4("u_ViewProj", camera.GetViewProjection());
///   sh.SetInt("u_Texture", 0);
///   // ... 绘制 ...
/// @ai_related SpriteBatcher, Texture
class Shader {
public:
    Shader();
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    /// @ai_summary 从两个 GLSL 源码字符串编译 + 链接。失败时打印错误并返回 false。
    bool LoadFromSource(const std::string& vsSource,
                        const std::string& fsSource);

    /// @ai_summary 释放 GL 程序对象。可重复调用。
    void Destroy();

    /// @ai_summary 在 GPU 上"使用"此 shader(后续 draw call 都用它)。
    void Bind() const;
    void Unbind() const;

    bool IsValid() const { return m_program != 0; }
    unsigned int GetProgramID() const { return m_program; }

    // Uniform 设置(自动绑定 shader,反复调用安全)
    void SetInt(const std::string& name, int v);
    void SetFloat(const std::string& name, float v);
    void SetVec2(const std::string& name, const Vec2& v);
    void SetVec3(const std::string& name, const Vec3& v);
    void SetVec4(const std::string& name, const Vec4& v);
    void SetMat4(const std::string& name, const Mat4& v);

private:
    int GetUniformLocation(const std::string& name);

    unsigned int m_program = 0;
    std::unordered_map<std::string, int> m_uniformCache;
};

}  // namespace AIForge
