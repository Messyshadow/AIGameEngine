#pragma once

#include <cmath>
#include <string>

namespace AIForge {

/// @ai_summary 二维向量（float），常用于UI/2D位置/纹理坐标。
/// @ai_example Vec2 v{1.0f, 2.0f};
/// @ai_related Vec3, Vec4
struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;
    Vec2(float xv, float yv) : x(xv), y(yv) {}
};

/// @ai_summary 三维向量（float），引擎所有3D坐标/欧拉角/缩放都用它。
/// @ai_example Vec3 pos{0, 1, 0};
/// @ai_example Vec3 rot{0, 90, 0};   // 欧拉角，单位是度
/// @ai_related Vec2, Vec4, Transform
struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vec3() = default;
    Vec3(float xv, float yv, float zv) : x(xv), y(yv), z(zv) {}

    /// @ai_summary 向量长度
    float Length() const { return std::sqrt(x * x + y * y + z * z); }

    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vec3 operator-() const { return {-x, -y, -z}; }
};

/// @ai_summary 向量点积
inline float Dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

/// @ai_summary 向量叉积(右手系)
inline Vec3 Cross(const Vec3& a, const Vec3& b) {
    return {a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
}

/// @ai_summary 归一化(零向量原样返回)
inline Vec3 Normalize(const Vec3& v) {
    float len = v.Length();
    if (len < 1e-8f) return v;
    return {v.x / len, v.y / len, v.z / len};
}

/// @ai_summary 四维向量（float），常用于颜色RGBA或四元数原始数据。
/// @ai_example Vec4 color{1, 0, 0, 1};   // 红色
/// @ai_related Vec3
struct Vec4 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;

    Vec4() = default;
    Vec4(float xv, float yv, float zv, float wv) : x(xv), y(yv), z(zv), w(wv) {}
};

/// @ai_summary 把"x,y,z"格式字符串解析为Vec3，失败时输出 (0,0,0) 并返回 false。
/// @ai_params text 形如 "1.0,2.0,3.0" 的字符串，允许空格。
/// @ai_params out 输出向量
/// @ai_example Vec3 v; if (ParseVec3("0,0,0", v)) { ... }
/// @ai_related Vec3
bool ParseVec3(const std::string& text, Vec3& out);

/// @ai_summary 4x4 浮点矩阵,列主序(column-major)对齐 GLSL 默认布局。
/// @ai_example
///   Mat4 proj = Mat4::Ortho(-100, 100, -100, 100, -1, 1);
///   Mat4 view = Mat4::Translate({-camX, -camY, 0});
///   Mat4 vp   = proj * view;
/// @ai_related Vec3, Camera2D
struct Mat4 {
    float m[16];

    Mat4();
    explicit Mat4(float diag);
    static Mat4 Identity();

    Vec4 operator*(const Vec4& v) const;
    Mat4 operator*(const Mat4& other) const;
    const float* Data() const { return m; }

    static Mat4 Translate(const Vec3& t);
    static Mat4 Scale(const Vec3& s);
    static Mat4 RotateZ(float radians);
    static Mat4 RotateX(float radians);
    static Mat4 RotateY(float radians);
    static Mat4 Ortho(float left, float right, float bottom, float top,
                      float znear, float zfar);
    static Mat4 Perspective(float fovYRadians, float aspect, float znear,
                            float zfar);

    /// @ai_summary 视图矩阵(摄像机看向某点)。3D 摄像机用。
    /// @ai_params eye    摄像机位置
    /// @ai_params target 注视点
    /// @ai_params up     上方向(通常 {0,1,0})
    static Mat4 LookAt(const Vec3& eye, const Vec3& target, const Vec3& up);
};

}  // namespace AIForge
