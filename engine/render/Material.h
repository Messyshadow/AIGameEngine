#pragma once

#include "../core/Math.h"

namespace AIForge {

/// @ai_summary PBR 材质参数(金属-粗糙度工作流)。
/// @ai_summary 这是描述"一个表面长什么样"的物理参数集 — 同一个着色器靠它表现
/// 金属/陶瓷/塑料/橡胶等所有材质。
/// @ai_example
///   Material gold;
///   gold.albedo    = {1.0f, 0.78f, 0.34f};
///   gold.metallic  = 1.0f;   // 纯金属
///   gold.roughness = 0.2f;   // 比较光滑
/// @ai_related Light, Mesh, Shader
struct Material {
    /// 基础色:非金属时是漫反射颜色,金属时是高光颜色
    Vec3  albedo{0.8f, 0.8f, 0.8f};

    /// 金属度:0 = 介电体(塑料/木/陶瓷),1 = 纯金属。通常二值。
    float metallic = 0.0f;

    /// 粗糙度:0 = 镜面光滑,1 = 完全漫反射
    float roughness = 0.5f;

    /// 环境光遮蔽:0 = 完全遮蔽(暗角/缝隙),1 = 不遮蔽
    float ao = 1.0f;

    /// 自发光颜色(灯管/岩浆等;{0,0,0} = 不发光)
    Vec3  emissive{0.0f, 0.0f, 0.0f};
};

}  // namespace AIForge
