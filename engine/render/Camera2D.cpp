#include "Camera2D.h"

namespace AIForge {

void Camera2D::SetViewport(int w, int h) {
    m_vpW = w > 0 ? w : 1;
    m_vpH = h > 0 ? h : 1;
}

Mat4 Camera2D::GetViewProjection() const {
    // 正交投影:把世界坐标映射到 [-1, 1] 的 NDC,viewport 像素是世界单位
    float halfW = (static_cast<float>(m_vpW) * 0.5f) / zoom;
    float halfH = (static_cast<float>(m_vpH) * 0.5f) / zoom;
    Mat4 proj = Mat4::Ortho(-halfW, halfW, -halfH, halfH, -1.0f, 1.0f);

    // 视图矩阵:摄像机的逆变换。摄像机右移 = 世界左移
    Mat4 view = Mat4::RotateZ(-rotation) *
                Mat4::Translate({-position.x, -position.y, 0.0f});

    return proj * view;
}

Vec2 Camera2D::ScreenToWorld(Vec2 s) const {
    // 屏幕像素(左上 0,0)→ 中心化(屏中央 0,0,Y 向上)→ /zoom + 相机位置
    float cx = s.x - static_cast<float>(m_vpW) * 0.5f;
    float cy = -(s.y - static_cast<float>(m_vpH) * 0.5f);
    return Vec2{cx / zoom + position.x, cy / zoom + position.y};
}

Vec2 Camera2D::WorldToScreen(Vec2 w) const {
    float cx = (w.x - position.x) * zoom;
    float cy = (w.y - position.y) * zoom;
    return Vec2{cx + static_cast<float>(m_vpW) * 0.5f,
                -cy + static_cast<float>(m_vpH) * 0.5f};
}

}  // namespace AIForge
