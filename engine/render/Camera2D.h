#pragma once

#include "../core/Math.h"

namespace AIForge {

/// @ai_summary 2D 正交摄像机:平移 + 缩放 + 旋转。
/// @ai_summary 1 单位 = 1 像素(zoom=1 时),(0,0) 是世界中心(屏幕中央)。
/// @ai_example
///   Camera2D cam;
///   cam.SetViewport(1280, 720);
///   cam.position = {100, 50};   // 摄像机向右上移
///   cam.zoom     = 2.0f;        // 放大 2 倍
///   Mat4 vp = cam.GetViewProjection();
///   shader.SetMat4("u_ViewProj", vp);
/// @ai_related Mat4, SpriteBatcher
class Camera2D {
public:
    Vec2  position{0.0f, 0.0f};
    float zoom     = 1.0f;
    float rotation = 0.0f;  // 弧度,绕屏幕中心

    /// @ai_summary 告诉摄像机当前 viewport 尺寸(像素)。窗口 resize 时调用。
    void SetViewport(int width, int height);

    int GetViewportWidth()  const { return m_vpW; }
    int GetViewportHeight() const { return m_vpH; }

    /// @ai_summary 计算 ViewProjection 矩阵(投影 × 视图)。
    Mat4 GetViewProjection() const;

    /// @ai_summary 屏幕坐标 → 世界坐标
    Vec2 ScreenToWorld(Vec2 screenPx) const;

    /// @ai_summary 世界坐标 → 屏幕坐标
    Vec2 WorldToScreen(Vec2 world) const;

private:
    int m_vpW = 1;
    int m_vpH = 1;
};

}  // namespace AIForge
