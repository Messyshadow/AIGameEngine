#pragma once

#include "../core/Math.h"

namespace AIForge {

/// @ai_summary 3D 透视摄像机,支持两种模式:
///   - Orbit:绕一个目标点旋转(看模型 / 编辑器常用)
///   - FPS:第一人称自由飞行(yaw/pitch + WASD)
/// @ai_summary 提供 View / Projection 矩阵给 shader 做 MVP 变换。
/// @ai_example
///   Camera3D cam;
///   cam.mode = Camera3D::Mode::Orbit;
///   cam.target = {0,0,0};
///   cam.distance = 12.0f;
///   Mat4 view = cam.GetView();
///   Mat4 proj = cam.GetProjection((float)winW / winH);
///   Mat4 mvp  = proj * view * model;
/// @ai_related Mat4, Mesh
class Camera3D {
public:
    enum class Mode { Orbit, FPS };

    Mode  mode        = Mode::Orbit;
    float fovDegrees  = 60.0f;
    float nearZ       = 0.1f;
    float farZ        = 500.0f;

    // ---- Orbit 模式参数 ----
    Vec3  target{0.0f, 0.0f, 0.0f};
    float distance    = 12.0f;
    float orbitYaw    = 0.7f;   // 弧度,绕 Y 轴
    float orbitPitch  = 0.5f;   // 弧度,俯仰

    // ---- FPS 模式参数 ----
    Vec3  fpsPosition{0.0f, 2.0f, 12.0f};
    float fpsYaw      = -1.5708f;  // 朝 -Z
    float fpsPitch    = 0.0f;

    /// @ai_summary 当前摄像机的世界坐标(eye)。
    Vec3 GetEyePosition() const;

    /// @ai_summary 视图矩阵。
    Mat4 GetView() const;

    /// @ai_summary 透视投影矩阵。
    /// @ai_params aspect 宽高比 = 窗口宽 / 窗口高
    Mat4 GetProjection(float aspect) const;

    /// @ai_summary 朝向单位向量(forward)。
    Vec3 GetForward() const;

    // ---- 交互辅助 ----

    /// @ai_summary Orbit:鼠标拖动旋转。dx/dy 是鼠标位移像素。
    void OrbitDrag(float dx, float dy, float sensitivity = 0.008f);

    /// @ai_summary 缩放:Orbit 改 distance,FPS 改 fov。delta 通常来自滚轮。
    void Zoom(float delta);

    /// @ai_summary FPS:鼠标看视角。
    void FPSLook(float dx, float dy, float sensitivity = 0.0035f);

    /// @ai_summary FPS:按本地方向移动(x=右, y=上, z=前)。
    void FPSMove(const Vec3& localDir, float dt, float speed);

private:
    void ClampPitch(float& pitch) const;
};

}  // namespace AIForge
