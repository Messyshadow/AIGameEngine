#include "Camera3D.h"

#include <algorithm>
#include <cmath>

namespace AIForge {

namespace {
constexpr float kPi        = 3.14159265358979f;
constexpr float kPitchLim  = 1.55334f;  // ~89 度,避免万向锁翻转
}  // namespace

void Camera3D::ClampPitch(float& pitch) const {
    if (pitch >  kPitchLim) pitch =  kPitchLim;
    if (pitch < -kPitchLim) pitch = -kPitchLim;
}

Vec3 Camera3D::GetEyePosition() const {
    if (mode == Mode::Orbit) {
        float cp = std::cos(orbitPitch);
        float sp = std::sin(orbitPitch);
        Vec3 offset{distance * cp * std::sin(orbitYaw),
                    distance * sp,
                    distance * cp * std::cos(orbitYaw)};
        return target + offset;
    }
    return fpsPosition;
}

Vec3 Camera3D::GetForward() const {
    if (mode == Mode::Orbit) {
        return Normalize(target - GetEyePosition());
    }
    float cp = std::cos(fpsPitch);
    return Normalize(Vec3{cp * std::cos(fpsYaw),
                          std::sin(fpsPitch),
                          cp * std::sin(fpsYaw)});
}

Mat4 Camera3D::GetView() const {
    Vec3 eye = GetEyePosition();
    Vec3 tgt = (mode == Mode::Orbit) ? target : (eye + GetForward());
    return Mat4::LookAt(eye, tgt, Vec3{0.0f, 1.0f, 0.0f});
}

Mat4 Camera3D::GetProjection(float aspect) const {
    if (aspect < 0.01f) aspect = 0.01f;
    float fovRad = fovDegrees * kPi / 180.0f;
    return Mat4::Perspective(fovRad, aspect, nearZ, farZ);
}

void Camera3D::OrbitDrag(float dx, float dy, float sensitivity) {
    orbitYaw   -= dx * sensitivity;
    orbitPitch += dy * sensitivity;
    ClampPitch(orbitPitch);
}

void Camera3D::Zoom(float delta) {
    if (mode == Mode::Orbit) {
        distance *= (1.0f - delta * 0.1f);
        distance = std::clamp(distance, 1.0f, 200.0f);
    } else {
        fovDegrees = std::clamp(fovDegrees - delta * 2.0f, 20.0f, 100.0f);
    }
}

void Camera3D::FPSLook(float dx, float dy, float sensitivity) {
    fpsYaw   += dx * sensitivity;
    fpsPitch -= dy * sensitivity;
    ClampPitch(fpsPitch);
}

void Camera3D::FPSMove(const Vec3& localDir, float dt, float speed) {
    Vec3 fwd   = GetForward();
    Vec3 right = Normalize(Cross(fwd, Vec3{0, 1, 0}));
    Vec3 up{0, 1, 0};
    Vec3 delta = right * localDir.x + up * localDir.y + fwd * localDir.z;
    fpsPosition = fpsPosition + delta * (speed * dt);
}

}  // namespace AIForge
