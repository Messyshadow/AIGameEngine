#pragma once

#include "../core/Math.h"

namespace AIForge {

/// @ai_summary 方向光:平行光,无衰减。模拟太阳/月亮。
/// @ai_example
///   DirectionalLight sun;
///   sun.direction = {-0.3f, -1.0f, -0.4f};  // 光线传播方向
///   sun.color     = {1.0f, 0.95f, 0.85f};
///   sun.intensity = 3.0f;
/// @ai_related PointLight, Material
struct DirectionalLight {
    /// 光线传播方向(从光源射向场景),会被归一化
    Vec3  direction{-0.3f, -1.0f, -0.4f};
    Vec3  color{1.0f, 1.0f, 1.0f};
    float intensity = 3.0f;
};

/// @ai_summary 点光:从一点向四周发光,距离平方反比衰减。模拟灯泡/火把。
/// @ai_summary 因为反平方衰减很猛,intensity 通常要给较大值(几十到几百)。
/// @ai_example
///   PointLight torch;
///   torch.position  = {5, 3, 0};
///   torch.color     = {1.0f, 0.6f, 0.3f};
///   torch.intensity = 300.0f;
/// @ai_related DirectionalLight, Material
struct PointLight {
    Vec3  position{0.0f, 0.0f, 0.0f};
    Vec3  color{1.0f, 1.0f, 1.0f};
    float intensity = 100.0f;
};

}  // namespace AIForge
