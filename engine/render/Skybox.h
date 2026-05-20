#pragma once

#include "../core/Math.h"
#include "Mesh.h"
#include "Shader.h"

namespace AIForge {

class Texture;

/// @ai_summary 天空盒渲染器:把一张 equirectangular HDR 当背景画在场景最远处。
/// @ai_summary 用 equirect 直采(方向 -> UV),不需要把 HDR 转成 cubemap,简单稳健。
/// @ai_summary 同一张 HDR 还能被 PBR 着色器拿去做 IBL 环境光/反射(见 Ch 09 demo)。
/// @ai_example
///   Texture env;  env.CreateFromHDR("data/textures/hdri/venice_sunset_1k.hdr");
///   Skybox sky;   sky.Init();
///   // 每帧(在画完不透明物体之后):
///   sky.Render(env, camera.GetView(), camera.GetProjection(aspect));
/// @ai_related Texture, Camera3D, Mesh
class Skybox {
public:
    Skybox() = default;

    /// @ai_summary 建好内部立方体网格 + 天空盒着色器(不含纹理)。
    bool Init();

    void Shutdown();

    /// @ai_summary 渲染天空背景。应在不透明几何体之后调用(深度=远平面)。
    /// @ai_params env  equirectangular HDR 纹理
    /// @ai_params view 摄像机视图矩阵(内部会去掉平移,让天空跟随相机)
    /// @ai_params proj 投影矩阵
    void Render(const Texture& env, const Mat4& view, const Mat4& proj);

    bool IsValid() const { return m_shader.IsValid(); }

private:
    Mesh   m_cube;
    Shader m_shader;
};

}  // namespace AIForge
