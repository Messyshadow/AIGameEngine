# Chapter 07 — 3D Mesh + Camera3D

> **本章定位**:跨过 2D/3D 的分界线。从这一章起,AIForge 进入三维世界。

---

## 一、本章意义

Ch 01-06 我们建好了完整的 2D 引擎。但你的目标里有只狼、怪猎、古惑狼、双人成行 —— **全是 3D**。

3D 渲染听起来玄,其实核心就一句话:

> **三维的点,乘上几个矩阵,变成屏幕上的二维像素。**

本章把这句话拆开讲清楚,然后画出第一个真正的 3D 物体:一群旋转的立方体,你能用摄像机绕着它们飞。

---

## 二、本章目标

| 目标 | 验收 |
|---|---|
| 理解 **MVP 矩阵**(Model-View-Projection) | 能说清每个矩阵干什么 |
| **透视投影** | 近大远小 |
| **Mesh** 类:3D 顶点缓冲 | 位置 + 法线 + 颜色 |
| **Camera3D**:Orbit + FPS 双模式 | 鼠标绕飞 / 第一人称 |
| **深度测试** | 前面的挡住后面的 |
| 简单方向光 | 立方体每个面亮度不同,看出立体感 |

跑出来:一个 3D 场景,地面 + 一圈旋转的彩色立方体 + 中心一个球,鼠标右键拖动绕场景飞。

---

## 三、核心:MVP 矩阵 —— 3D 渲染的全部数学

一个顶点从"模型自己的坐标"到"屏幕坐标",要经过三次变换:

```
模型空间          世界空间          摄像机空间         裁剪空间
(立方体自己) ──M──→ (放进世界) ──V──→ (从相机看) ──P──→ (投影到屏幕)
            Model           View          Projection

最终: gl_Position = P * V * M * vec4(顶点, 1.0)
              简写 = MVP * 顶点
```

| 矩阵 | 全名 | 干什么 | 例子 |
|---|---|---|---|
| **M** | Model | 把模型摆到世界里的某个位置/旋转/缩放 | "立方体放在 (5,0,3),转 45°" |
| **V** | View | 把世界变换到"以摄像机为原点"的坐标系 | "相机在哪、看哪" |
| **P** | Projection | 把 3D 锥形视野压扁成屏幕矩形 | "60° 视野,近大远小" |

**关键认知**:GPU 不"理解"3D。它只会做"顶点 × 矩阵"。3D 的全部魔法,就是把摆放、观察、投影这三件事编码进 3 个 4×4 矩阵,相乘一次搞定。

---

## 四、透视投影 vs 正交投影

| | 正交(Ch 04 Camera2D) | 透视(Ch 07 Camera3D) |
|---|---|---|
| 形状 | 长方体视野 | **锥形(平截头体 Frustum)视野** |
| 远近 | 远近一样大 | **近大远小** |
| 用途 | 2D 游戏 / UI / 工程图 | 3D 游戏 / 第一人称 |

透视投影矩阵的核心参数:
- **fov**(Field of View,视野角度):60° 是标准,90° 广角,30° 望远
- **aspect**:宽高比 = 窗口宽 / 高
- **near / far**:近裁剪面 / 远裁剪面 —— 比 near 近、比 far 远的东西不画

⚠️ **near 不能设太小**(如 0.001),否则深度精度爆炸,出现 z-fighting(物体表面闪烁)。

---

## 五、视图矩阵 / LookAt

摄像机本身也是用矩阵表达的。`LookAt(eye, target, up)` 生成视图矩阵:
- **eye**:摄像机在哪
- **target**:看向哪个点
- **up**:哪边是"上"(通常 {0,1,0})

原理:View 矩阵其实是"摄像机变换的逆" —— 与其移动相机,不如反向移动整个世界。相机右移 = 世界左移。

本章给 `Mat4` 加了 `LookAt`,给 `Vec3` 加了 `Dot / Cross / Normalize`(LookAt 要用叉积算相机的右/上方向)。

---

## 六、Mesh —— 3D 物体的数据

一个 3D 物体 = **一堆顶点 + 一堆索引**。

```cpp
struct Vertex {
    Vec3 pos;     // 位置
    Vec3 normal;  // 法线(指向表面外侧,光照要用)
    Vec3 color;   // 颜色
};
```

**法线(Normal)**是新东西:它是垂直于表面、指向外侧的单位向量。光照计算靠它 —— 表面法线和光线方向的夹角,决定这个面有多亮。

一个立方体 = **24 个顶点**(6 面 × 4 角,每面顶点带自己的法线)+ **36 个索引**(6 面 × 2 三角形 × 3)。

> 为什么不是 8 个顶点?因为每个角属于 3 个面,3 个面法线不同,所以要拆成 24 个。

---

## 七、深度测试 —— 谁挡住谁

2D 靠绘制顺序(后画的盖前面的)。3D 不行 —— 物体在空间里有前后,不能靠顺序。

**深度缓冲(Depth Buffer)**:每个像素除了颜色,还存一个"深度值"(离相机多远)。画新像素前先比深度:更近才画,更远丢弃。

```cpp
glEnable(GL_DEPTH_TEST);   // 开启
// 每帧 glClear(GL_DEPTH_BUFFER_BIT) 清掉上一帧深度
```

窗口创建时要申请深度缓冲(`SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24)`)—— AIForge 的 Window 已经做了。

---

## 八、Camera3D 双模式

| 模式 | 操作 | 用途 |
|---|---|---|
| **Orbit** | 绕一个目标点转(yaw/pitch + distance) | 看模型、编辑器、Boss 展示 |
| **FPS** | position + yaw/pitch,WASD 走 | 第一人称、自由飞行 |

本章 demo 默认 Orbit(鼠标右键拖动绕飞),按 **F** 切换到 FPS 自由飞行。

---

## 九、新增文件

| 文件 | 内容 |
|---|---|
| `engine/core/Math.h/cpp` | 加 Vec3 运算 + `Dot/Cross/Normalize` + `Mat4::LookAt` |
| `engine/render/Mesh.h/cpp` | 3D 网格 + `MeshFactory`(Cube/Plane/Sphere) |
| `engine/render/Camera3D.h/cpp` | 透视摄像机,Orbit + FPS |
| `examples/07_*/main.cpp` | demo:旋转立方体阵 + 地面 + 球 + 摄像机 |

---

## 十、Demo 体验

`chapter_07_demo.exe`:
- 一个 3D 场景:**灰色地面** + **一圈 8 个旋转的彩色立方体** + **中心一个大球**
- 一束方向光(从右上方打下),所以立方体每个面亮度不同 —— 有立体感
- **控制**:
  | 操作 | 效果 |
  |---|---|
  | 鼠标**右键拖动** | Orbit 绕场景旋转视角 |
  | **滚轮** | 拉近 / 拉远 |
  | **WASD** | 平移注视点(Orbit)/ 移动(FPS) |
  | **F** | 切换 Orbit / FPS 模式 |
  | **ESC** | 退出 |
- 标题栏:`mode + camera pos + FPS`

---

## 十一、课后练习

1. 给 `MeshFactory` 加 `BuildCylinder` / `BuildCone`
2. 实现 **视锥裁剪**:摄像机看不到的立方体不画(标题栏显示剔除了几个)
3. FPS 模式加重力 + 跳跃,变成简单的"在地面上走"
4. 给立方体加纹理(扩展 `Mesh::Vertex` 加 UV,复用 Ch 04 的 Texture)

---

## 十二、常见坑

- **忘了开深度测试** → 远处物体盖住近处,画面混乱
- **near 太小** → z-fighting(表面闪烁)
- **矩阵乘法顺序** → `P*V*M` 不是 `M*V*P`,顺序错了画面全乱
- **法线没归一化** → 光照强度不对
- **背面剔除**:本章故意 **不开** `GL_CULL_FACE`(省事);开了能省一半三角形,但要保证顶点绕序一致 —— 留作优化

---

## 十三、延伸阅读

- [LearnOpenGL — Coordinate Systems](https://learnopengl.com/Getting-started/Coordinate-Systems)
- [LearnOpenGL — Camera](https://learnopengl.com/Getting-started/Camera)
- [3Blue1Brown — 线性代数本质](https://www.3blue1brown.com/topics/linear-algebra)(直观理解矩阵)
- [Scratchapixel — Perspective Projection](https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix.html)

---

## 十四、下一章预告

**Ch 08 — PBR 材质 + 光照**:立方体现在是纯色的。下一章上 **基于物理的渲染(PBR)** —— 金属、木头、塑料各有质感的材质球,Cook-Torrance BRDF,这是现代 3D 引擎的画质基石。
