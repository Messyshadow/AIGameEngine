# Chapter 07 — 3D Mesh + Camera3D

> **本章定位**:进入 3D 世界。

---

## 本章目标
1. 3D 数学:`Mat4`、四元数 `Quat`、欧拉角转换
2. **MVP(Model-View-Projection)矩阵**理解
3. **透视投影**:fov / aspect / near / far
4. Mesh 类:顶点 + 索引 + 属性布局
5. 三种摄像机模式:**FPS / Orbit / TPS**
6. **Frustum Culling**(视锥裁剪)入门
7. Demo:5 个旋转的彩色立方体,鼠标控制摄像机

## 跑出来的 Demo
`build/Release/chapter_07_demo.exe` — 3D 场景里 5 个旋转立方体,鼠标右键拖动旋转视角,WASD 移动,滚轮拉近拉远。屏幕左上显示当前可见物体数 / 被裁剪数。

## 学到的前沿技术
- **MVP 矩阵**:为什么 GPU 喜欢矩阵 — 一次乘法搞定平移+旋转+缩放+投影
- **行优先 vs 列优先**:GLSL 默认列优先,C++ 实现要小心
- **四元数 vs 欧拉角**:为什么旋转用四元数避免万向锁(Gimbal Lock)
- **Frustum 6 平面剔除**:盒子/球体快速判定
- **深度测试 / 深度缓冲**:为什么前面挡住后面

## 背景知识

3D 渲染的全部数学其实就是:**点 × 矩阵 = 屏幕坐标**。MVP 把"模型局部坐标 → 世界坐标 → 摄像机坐标 → 裁剪坐标"这一连串变换打包成 4×4 矩阵相乘。

四元数(Quaternion)是 3D 旋转的工业标准 — 紧凑(4 个 float)、可插值(SLERP)、无万向锁。Unity / Unreal / Godot 内部全是四元数,只在 Inspector 里显示成欧拉角给人看。

视锥裁剪是性能优化第一步:屏幕看不到的物体根本不要画。AAA 引擎还有 Occlusion Culling(被遮挡裁剪),那是 Ch 13 的进阶。

## 架构设计

```
Mesh
 ├─ VBO(顶点)
 ├─ EBO(索引)
 ├─ Layout(pos/normal/uv/tangent...)
 └─ Bounds(AABB / Sphere)

Camera3D(基类)
 └─ ProjectionMatrix() / ViewMatrix()

Camera3D 实现:
 ├─ FPSCamera (yaw/pitch + 第一人称鼠标视角)
 ├─ OrbitCamera (绕中心旋转,带距离)
 └─ TPSCamera (跟随目标,有偏移)

Frustum
 ├─ ExtractFromVP(Mat4)
 └─ Test(AABB) → INSIDE/OUTSIDE/INTERSECT
```

## 涉及源文件
- `engine/core/Math.h/cpp` — 扩展 Mat4 / Quat
- `engine/render/Mesh.h/cpp`
- `engine/render/Camera.h/cpp`
- `engine/render/Frustum.h/cpp`
- `data/shaders/standard.vs/fs`(基础版)

## 课后练习
1. 加 `MeshFactory::CreateCube/Sphere/Plane`
2. 让 OrbitCamera 支持平滑减速
3. 实现 AABB Frustum Culling

## 常见坑
- 透视投影矩阵:near 太小会深度精度爆炸(z-fighting)
- 四元数乘法顺序:`q1 * q2` 是"先 q2 后 q1"还是反过来?GLM / 自写要统一
- 深度测试要开 `glEnable(GL_DEPTH_TEST)`,且 FBO 要有 depth attachment

## 延伸阅读
- [3Blue1Brown — Linear Algebra Series](https://www.3blue1brown.com/topics/linear-algebra) — 直观理解矩阵
- [Real-Time Rendering 4th Edition](https://www.realtimerendering.com/) — 第 2-4 章,Bible
- [LearnOpenGL — Coordinate Systems](https://learnopengl.com/Getting-started/Coordinate-Systems)
- [Bullet Physics — Geometric Tools](https://www.geometrictools.com/) — Frustum 裁剪经典实现

## 下一章预告
**Ch 08 — PBR 材质 + 光照**:画上去的不再是纯色立方体,而是金属、木头、砖块各有质感的材质球。
