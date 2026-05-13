# Chapter 04 — 2D Sprite Batcher + Camera2D

> **本章定位**:把第一个真正的"游戏内容"画到屏幕上。引擎从"会跑"升级到"能画"。

---

## 一、本章为什么重要

到目前为止(Ch 01-03):
- Ch 01:窗口 + GL Context
- Ch 02:ECS + 用 GL_POINTS 画了一些圆点(临时 hack,在 main.cpp 里)
- Ch 03:命令系统 + AIContext

**Ch 04 之前,引擎本体里没有任何"绘图"代码**。`engine/render/` 目录是空的。Ch 04 之后,引擎就有了**真正的 2D 渲染管线**,能撑起一个完整的 2D 游戏(类 Vampire Survivors / Brotato)。

学完本章你将拥有的能力:
- **5000+ 个独立 sprite 在屏幕上动,锁 60 FPS**
- 摄像机平移 / 缩放 / 旋转
- 一个工业标准的**批渲染(Batch Rendering)**系统
- 真正理解为什么"减少 draw call"是 2D 游戏性能的圣经

---

## 二、本章目标(具体可测)

| 目标 | 验收标准 |
|---|---|
| 屏幕上画 5000 sprite | 至少 5000 个独立位置/颜色的彩色方块,**单 draw call** |
| 60 FPS 稳定 | 240Hz 屏幕上锁 240 FPS,不掉帧 |
| 摄像机交互 | WASD 平移,Q/E 缩放 |
| 实时统计 | 标题栏显示 sprite 数 / draw call 数 / FPS |
| 程序化纹理 | 软圆形 alpha 渐变,无 PNG 依赖(stb_image 加载留给 Ch 05) |
| RenderAPI 雏形 | 不抽抽象基类(过早抽象),但代码结构便于 Ch 23 加 Vulkan |

---

## 三、行业故事 — 批渲染是怎么救命的

### 3.1 朴素 2D 渲染:每个 sprite 一次 draw call

```cpp
for (auto& s : sprites) {
    BindShader(shader);
    BindTexture(s.texture);
    UploadUniforms(s.pos, s.size, s.color);
    DrawTriangles(quad_vbo);   // 一次 GPU 调用
}
```

**问题**:1000 个 sprite = 1000 次 CPU↔GPU 通信。**瓶颈在 CPU**(GPU 闲着),帧率被驱动开销拖死。一个独立游戏团队踩这个坑能让他们花 3 个月优化。

### 3.2 Vampire Survivors 怎么做到屏幕 5000+ 单位 60FPS?

[GDC 2024 演讲](https://www.youtube.com/watch?v=H2eoB6V_5LU) 揭秘:**所有敌人在 1 个 draw call 里画完**。秘密就是**批渲染(Batch Rendering)**:
- 一次性把所有 sprite 的顶点数据上传到一个大 VBO
- 一次 `glDrawArrays` 把整批画完
- CPU↔GPU 通信次数从 5000 降到 **1**

[Brotato](https://store.steampowered.com/app/1942280/Brotato/) / [HoloCure](https://store.steampowered.com/app/2420510/HoloCure__Save_the_Fans/) 同款套路。

### 3.3 主流引擎都做了

| 引擎 | 批渲染叫法 |
|---|---|
| Unity URP/HDRP | **SRP Batcher** + GPU Instancing |
| Unreal | **Auto-Instancing** + Hierarchical LOD |
| Godot 4 | **MultiMesh** + Canvas Item Batching |
| libGDX | **SpriteBatch** ⭐ AIForge 主要参考 |
| Hazel | **Renderer2D** ⭐ 我们参考它的代码结构 |
| **AIForge** | **SpriteBatcher**(我们写的) |

---

## 四、OpenGL 渲染管线速成(避免 Ch 04 完全黑箱)

### 4.1 数据流(从 CPU 到屏幕)

```
你的 C++ 代码
   ↓ 把顶点数据塞进 VBO(Vertex Buffer Object)
GPU 顶点输入装配
   ↓ 按顶点逐个执行 Vertex Shader
GPU 顶点处理(变换到屏幕坐标系)
   ↓ 装配成三角形 + 光栅化(每个三角形变成像素覆盖)
GPU 片段处理
   ↓ 每个像素执行 Fragment Shader
帧缓冲(Framebuffer)
   ↓ SwapBuffers
屏幕
```

### 4.2 三个核心对象

| 对象 | 全名 | 作用 |
|---|---|---|
| **VBO** | Vertex Buffer Object | 装顶点数据的 GPU 内存块 |
| **EBO/IBO** | Element/Index Buffer Object | 装"哪几个顶点组成三角形"的索引数据 |
| **VAO** | Vertex Array Object | "记忆"VBO 的格式(哪几个 float 是 position,哪几个是颜色...) |

### 4.3 一个 sprite = 4 顶点 + 6 索引

```
顶点(左下、右下、右上、左上):
0 (-0.5, -0.5)    1 (0.5, -0.5)    2 (0.5, 0.5)    3 (-0.5, 0.5)

索引(2 个三角形,共用顶点):
0,1,2  ← 右下三角
2,3,0  ← 左上三角

总数据(一个 sprite):4 顶点 × 8 float = 32 float ≈ 128 字节
                    6 索引 × 4 byte = 24 字节
```

**5000 sprite 一帧约 640 KB**,对 GPU 是小儿科(显存按 GB 算)。

---

## 五、AIForge SpriteBatcher 设计

### 5.1 整体架构

```
SpriteBatcher
 ├─ Init()
 │   ├─ 创建一个大 VBO(预分配 MAX_SPRITES × 4 顶点的空间)
 │   ├─ 创建 EBO(预填好的 quad 索引)
 │   ├─ 创建 VAO(绑定 VBO + 顶点属性布局)
 │   └─ 编译 sprite.vs / sprite.fs
 │
 ├─ Begin(camera, texture)
 │   ├─ 清空本次提交的顶点缓存
 │   ├─ 记录摄像机矩阵 + 纹理
 │   └─ 准备接收 Submit 调用
 │
 ├─ Submit(pos, size, color, rotation)
 │   └─ 把 4 个顶点(应用 size + rotation + pos)追加到顶点缓存
 │
 └─ End()
     ├─ 把顶点缓存 glBufferSubData 一次性上传
     ├─ Shader.Bind() + 设置 viewProj uniform
     ├─ Texture.Bind()
     ├─ glDrawElements(GL_TRIANGLES, sprite_count * 6)  ← 一次 draw call!
     └─ 统计 draw_calls++ / sprites_drawn += N
```

### 5.2 顶点格式(本章简化版 — 单纹理)

```cpp
struct SpriteVertex {
    Vec2 pos;     // 世界坐标(已应用 size + rotation + offset)
    Vec2 uv;      // 纹理坐标 [0,1]
    Vec4 color;   // 顶点色(RGBA),与纹理相乘
};
// 8 floats / 顶点
```

Ch 05 会扩展:加 `float textureIndex` 以支持**多纹理批渲染**(`sampler2D u_Textures[32]`)。

### 5.3 GLSL 着色器(完整代码)

**sprite.vs**:
```glsl
#version 450 core
layout(location=0) in vec2 a_Pos;
layout(location=1) in vec2 a_UV;
layout(location=2) in vec4 a_Color;

uniform mat4 u_ViewProj;

out vec2 v_UV;
out vec4 v_Color;

void main() {
    gl_Position = u_ViewProj * vec4(a_Pos, 0.0, 1.0);
    v_UV = a_UV;
    v_Color = a_Color;
}
```

**sprite.fs**:
```glsl
#version 450 core
in vec2 v_UV;
in vec4 v_Color;
out vec4 frag;

uniform sampler2D u_Texture;

void main() {
    vec4 t = texture(u_Texture, v_UV);
    frag = t * v_Color;
    if (frag.a < 0.01) discard;
}
```

---

## 六、Camera2D 设计

**正交投影**(2D 游戏专用,无近大远小):

```cpp
class Camera2D {
public:
    Vec2  position = {0, 0};
    float zoom     = 1.0f;
    float rotation = 0.0f;  // 弧度

    void   SetViewport(int width, int height);
    Mat4   GetViewProjection() const;
};

Mat4 Camera2D::GetViewProjection() const {
    float halfW = (m_vpW * 0.5f) / zoom;
    float halfH = (m_vpH * 0.5f) / zoom;
    Mat4 proj = Mat4::Ortho(-halfW, halfW, -halfH, halfH, -1, 1);
    Mat4 view = Mat4::Translate({-position.x, -position.y, 0}) *
                Mat4::RotateZ(-rotation);
    return proj * view;
}
```

**正交投影矩阵 vs 透视投影矩阵**:
- **正交**(2D 用):平行投影,远近物体一样大。世界坐标直接映射到屏幕格子。
- **透视**(3D 用):锥体投影,远小近大。Ch 07 引入。

---

## 七、本章涉及的文件

| 文件 | 状态 |
|---|---|
| `engine/core/Math.h/cpp` | 🔧 增强:加 `Mat4` 类 + Ortho/Translate/Scale/RotateZ 工厂 |
| `engine/render/Shader.h/cpp` | 🆕 GLSL 编译 / 链接 / 设置 uniform |
| `engine/render/Texture.h/cpp` | 🆕 OpenGL 纹理对象(支持从 raw RGBA 创建,本章用) |
| `engine/render/Camera2D.h/cpp` | 🆕 正交摄像机 |
| `engine/render/SpriteBatcher.h/cpp` | 🆕 ★ 本章核心 |
| `examples/04_2d_sprite_batcher/main.cpp` | 🆕 5000 sprite 弹球 demo |

---

## 八、设计思考 — 为什么这么写

| 选择 | 理由 |
|---|---|
| **暂不抽 RenderAPI 基类** | 只有 OpenGL 一个后端,抽象层是负担。Ch 23 加 Vulkan 时再抽 |
| **单纹理批渲染**(本章) | 简单清晰,先把"批"的核心说清楚。多纹理 atlas 留给 Ch 05 |
| **顶点数据 CPU 算后上传**(不用 instancing) | instancing 性能略好但代码复杂,且单 draw call 5000 sprite 已经远超需求 |
| **程序化纹理**(无 PNG 加载) | 把 stb_image / 文件 IO 留给 Ch 05,本章只讲渲染管线 |
| **正交投影固定到屏幕像素** | 1 单位 = 1 像素,直觉好理解。Ch 06 引入 world unit 转换 |

---

## 九、Demo 体验

`chapter_04_demo.exe` 启动后:
- **窗口**:1280×720,黑色背景
- **5000 个软圆形 sprite**,每个有随机位置/速度/颜色/大小
- 它们在屏幕内**弹来弹去**(撞到边界反弹)
- **WASD**:平移摄像机
- **Q / E**:缩小 / 放大
- **R**:重置摄像机
- **ESC**:退出
- **标题栏**:`sprites=5000 | draw_calls=1 | FPS=240`

**关键看点**:
- `draw_calls=1` — 这是核心成就,1 次 GPU 调用画完所有 sprite
- 帧率不会因 sprite 数变化(瓶颈是 GPU 填充,不是 CPU 调度)

---

## 十、课后练习

1. 把 sprite 数从 5000 改成 50000,看 FPS 是否降(预期:轻微降,因为 GPU 填充率变成瓶颈)
2. 加一个新 sprite 类型,用第二张程序化纹理(此时 draw_call=2,因为不同纹理要切换)
3. 实现 `Submit` 时按 z 值排序,让"上层"sprite 后画
4. 实现 `Camera2D::WorldToScreen` 和 `ScreenToWorld` 互转

---

## 十一、常见坑

- **VBO 大小不够**:`MAX_SPRITES` 预设 10000,超过会断言或裁剪
- **`glBufferSubData` vs `glBufferData`**:用前者(数据动态变,空间已分配);用后者会每帧重新分配显存
- **Y 轴方向**:屏幕 Y 朝下 vs 世界 Y 朝上 — 我们用"Y 朝上"(数学习惯),投影矩阵处理翻转
- **混合模式**:默认 `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`,直接用 alpha 透明。HDR 时改用 premultiplied alpha
- **纹理黑边**:边缘像素的 alpha 渐变要 premultiply,否则会有黑色光晕

---

## 十二、延伸阅读

- [LearnOpenGL — Textures](https://learnopengl.com/Getting-started/Textures)
- [The Cherno — Hazel Renderer2D](https://www.youtube.com/playlist?list=PLlrATfBNZ98fqE45g3jZA_hLGUrD4bo6_)(批渲染视频教程)
- [Vampire Survivors GDC Talk](https://www.youtube.com/watch?v=H2eoB6V_5LU)(数千实体怎么 60fps)
- [libGDX SpriteBatch Source](https://github.com/libgdx/libgdx/blob/master/gdx/src/com/badlogic/gdx/graphics/g2d/SpriteBatch.java)(工业级实现)
- [GPU Gems 1 - Chapter 28: Graphics Pipeline Performance](https://developer.nvidia.com/gpugems/gpugems/part-v-performance-and-practicalities/chapter-28-graphics-pipeline-performance)

---

## 十三、下一章预告

**Ch 05 — 2D 动画 + 粒子 + Tilemap**:有了批渲染,下一步加 sprite sheet 动画 + 粒子系统(爆炸/拖尾)+ 瓦片地图。**这一章结束后,我们就能做出 Vampire Survivors 的雏形**。

后面 Ch 06 加 2D 光照 / Bloom,Ch 13 上 GPU 百万粒子 / 爆炸冲击波,Ch 13.5 你提议的 GPU 流体仿真。

---

## 实施清单

- [ ] `Mat4` 类(engine/core/Math.h/cpp)
- [ ] `Shader` 类(GLSL 编译 + uniform 设置)
- [ ] `Texture` 类(从 raw RGBA 创建)
- [ ] `Camera2D` 类(ortho + zoom + rotation)
- [ ] `SpriteBatcher` 类(本章核心)
- [ ] Demo:5000 弹跳 sprite + WASD 摄像机
- [ ] CMakeLists 加 `engine/render/*` glob + `chapter_04_demo` target
- [ ] 编译 + 验证 draw_calls = 1
