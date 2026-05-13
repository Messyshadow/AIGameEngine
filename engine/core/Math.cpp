#include "Math.h"

#include <cstdlib>
#include <cstring>

namespace AIForge {

// ============================================================================
//  Mat4 实现 - 列主序(column-major,匹配 GLSL 默认)
//
//  存储布局:
//    m[ 0] m[ 4] m[ 8] m[12]
//    m[ 1] m[ 5] m[ 9] m[13]      ← 行
//    m[ 2] m[ 6] m[10] m[14]
//    m[ 3] m[ 7] m[11] m[15]
//         ↑ 列
//  访问元素 (row, col):m[col*4 + row]
// ============================================================================

Mat4::Mat4() {
    for (int i = 0; i < 16; ++i) m[i] = 0.0f;
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

Mat4::Mat4(float diag) {
    for (int i = 0; i < 16; ++i) m[i] = 0.0f;
    m[0] = m[5] = m[10] = m[15] = diag;
}

Mat4 Mat4::Identity() { return Mat4(); }

Vec4 Mat4::operator*(const Vec4& v) const {
    Vec4 r;
    r.x = m[0] * v.x + m[4] * v.y + m[8]  * v.z + m[12] * v.w;
    r.y = m[1] * v.x + m[5] * v.y + m[9]  * v.z + m[13] * v.w;
    r.z = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * v.w;
    r.w = m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15] * v.w;
    return r;
}

Mat4 Mat4::operator*(const Mat4& o) const {
    Mat4 r(0.0f);
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            float sum = 0;
            for (int k = 0; k < 4; ++k) {
                sum += m[k * 4 + row] * o.m[col * 4 + k];
            }
            r.m[col * 4 + row] = sum;
        }
    }
    return r;
}

Mat4 Mat4::Translate(const Vec3& t) {
    Mat4 r;  // 单位阵
    r.m[12] = t.x;
    r.m[13] = t.y;
    r.m[14] = t.z;
    return r;
}

Mat4 Mat4::Scale(const Vec3& s) {
    Mat4 r(0.0f);
    r.m[0]  = s.x;
    r.m[5]  = s.y;
    r.m[10] = s.z;
    r.m[15] = 1.0f;
    return r;
}

Mat4 Mat4::RotateZ(float rad) {
    Mat4 r;
    float c = std::cos(rad), s = std::sin(rad);
    r.m[0] =  c; r.m[4] = -s;
    r.m[1] =  s; r.m[5] =  c;
    return r;
}

Mat4 Mat4::RotateX(float rad) {
    Mat4 r;
    float c = std::cos(rad), s = std::sin(rad);
    r.m[5]  =  c; r.m[9]  = -s;
    r.m[6]  =  s; r.m[10] =  c;
    return r;
}

Mat4 Mat4::RotateY(float rad) {
    Mat4 r;
    float c = std::cos(rad), s = std::sin(rad);
    r.m[0]  =  c; r.m[8]  =  s;
    r.m[2]  = -s; r.m[10] =  c;
    return r;
}

Mat4 Mat4::Ortho(float left, float right, float bottom, float top,
                 float znear, float zfar) {
    Mat4 r(0.0f);
    r.m[0]  =  2.0f / (right - left);
    r.m[5]  =  2.0f / (top - bottom);
    r.m[10] = -2.0f / (zfar - znear);
    r.m[12] = -(right + left)  / (right - left);
    r.m[13] = -(top + bottom)  / (top - bottom);
    r.m[14] = -(zfar + znear)  / (zfar - znear);
    r.m[15] =  1.0f;
    return r;
}

Mat4 Mat4::Perspective(float fovYRad, float aspect, float znear, float zfar) {
    Mat4 r(0.0f);
    float f = 1.0f / std::tan(fovYRad * 0.5f);
    r.m[0]  = f / aspect;
    r.m[5]  = f;
    r.m[10] = (zfar + znear) / (znear - zfar);
    r.m[11] = -1.0f;
    r.m[14] = (2.0f * zfar * znear) / (znear - zfar);
    return r;
}

bool ParseVec3(const std::string& text, Vec3& out) {
    out = Vec3{};
    float values[3] = {0, 0, 0};
    int idx = 0;
    const char* p = text.c_str();
    while (*p && idx < 3) {
        while (*p == ' ' || *p == '\t') ++p;
        char* end = nullptr;
        float v = std::strtof(p, &end);
        if (end == p) return false;
        values[idx++] = v;
        p = end;
        while (*p == ' ' || *p == '\t') ++p;
        if (*p == ',') ++p;
    }
    if (idx != 3) return false;
    out = Vec3{values[0], values[1], values[2]};
    return true;
}

}  // namespace AIForge
