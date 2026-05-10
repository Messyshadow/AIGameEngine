# Chapter 04 — 2D Sprite Batcher + Camera2D

> **本章定位**:把第一个像素画到屏幕上 — 现代 2D 渲染管线。

---

## 本章目标
1. 创建 VBO / VAO / EBO,理解顶点输入流水线
2. 编写第一个 GLSL Shader(顶点 + 片段)
3. 加载 PNG 纹理(stb_image)
4. **Sprite Batcher**:把上千个 sprite 合并到单个 draw call
5. 正交投影 + Camera2D(平移 / 缩放)
6. 屏幕上 1 万个移动 sprite 稳定 60fps

## 跑出来的 Demo
`build/Release/chapter_04_demo.exe` — 屏幕上 10000 个彩色小方块(或 sprite)朝随机方向移动,左下角显示 FPS / Draw Call 数(应该是 1)。WASD 控制摄像机,滚轮缩放。

## 学到的前沿技术
- **VBO / VAO / EBO**:OpenGL 把数据传给 GPU 的标准方式
- **Vertex / Fragment Shader 流水线**:GPU 怎么把三角形变成像素
- **正交投影矩阵**:2D 渲染的"摄像机"
- **Texture Atlas**(贴图集):把多张小图打包成大图,降低绑定切换
- **批渲染(Batch Rendering)**:为什么单 draw call 1 万 sprite 比 1 万次 draw call 快 100 倍
- **Indirect Draw 入门**(Ch 13 详细讲)

## 背景知识(为什么这章必要)

朴素 2D 渲染:每个 sprite 一次 draw call,1000 个 sprite = 1000 次 CPU↔GPU 通信。**瓶颈在 CPU**。

现代游戏引擎(Unity SpriteRenderer / Unreal Paper2D / Godot CanvasItem)都做"自动批处理":把同纹理 + 同 shader 的 sprite 合并成一个大 mesh,一次 draw 完。

行业典型:Vampire Survivors 屏幕上 5000+ 单位还能 60fps,关键就是 batcher。本引擎的 SpriteBatcher 直接对标这个标准。

## 架构设计

```
SpriteBatcher
 ├─ 一个大 VBO(动态)
 │  └─ 每帧 BeginFrame 清空,Submit 写入
 ├─ DrawCall:
 │  └─ 当遇到不同纹理/shader/blend mode 时,提交并新开一批
 ├─ 数据结构:
 │     struct SpriteVertex { Vec2 pos; Vec2 uv; Vec4 color; uint32_t texIdx; }
 └─ EndFrame → 提交剩余批次

Camera2D
 ├─ position: Vec2
 ├─ zoom: float
 ├─ rotation: float
 └─ ProjectionView() → Mat4

RenderAPI(抽象基类)
 ├─ Init(window)
 ├─ BeginFrame() / EndFrame()
 ├─ DrawSprite(tex, pos, size, color, ...)
 └─ Future: DrawMesh / DrawSkybox(Ch 07+)
```

## 涉及源文件
- `engine/render/RenderAPI.h` — 渲染抽象基类
- `engine/render/backend_gl/SpriteBatcher.h/cpp` — 批渲染
- `engine/render/backend_gl/Shader.h/cpp` — Shader 加载
- `engine/render/backend_gl/Texture.h/cpp` — stb_image 纹理
- `engine/render/Camera.h` — Camera2D / Camera3D
- `data/shaders/sprite.vs/fs` — sprite 着色器

## Shader 关键代码示意

**sprite.vs**:
```glsl
#version 450 core
layout(location=0) in vec2 a_Pos;
layout(location=1) in vec2 a_UV;
layout(location=2) in vec4 a_Color;
layout(location=3) in float a_TexIdx;

uniform mat4 u_ViewProj;

out vec2 v_UV;
out vec4 v_Color;
out float v_TexIdx;

void main() {
    gl_Position = u_ViewProj * vec4(a_Pos, 0.0, 1.0);
    v_UV = a_UV;
    v_Color = a_Color;
    v_TexIdx = a_TexIdx;
}
```

**sprite.fs**:
```glsl
#version 450 core
in vec2 v_UV;
in vec4 v_Color;
in float v_TexIdx;

uniform sampler2D u_Textures[32];

out vec4 frag;
void main() {
    int idx = int(v_TexIdx);
    frag = texture(u_Textures[idx], v_UV) * v_Color;
}
```

## 实施清单
- [ ] OpenGL VBO/VAO/EBO 抽象
- [ ] Shader 类(LoadFromFile / Bind / SetUniform)
- [ ] Texture 类(stb_image 加载)
- [ ] SpriteBatcher 类
- [ ] Camera2D 类
- [ ] sprite.vs/fs
- [ ] chapter_04_demo:1 万 sprite 示例
- [ ] Draw call 计数器

## 课后练习
1. 让 sprite 支持旋转(顶点变换矩阵)
2. 把 sprite 颜色每帧渐变(顶点色 + sin 波)
3. 加载真实 PNG(从 mini_game_engine/data/texture 拿一张)替代纯色

## 常见坑
- Texture 数量上限:GL 支持 ≥ 16,但实际 array sampler 写法在 AMD 旧驱动可能崩,改 bindless 或限制到 16
- Y 轴方向:OpenGL 默认 Y 向上,屏幕坐标 Y 向下,要在投影矩阵处理
- Premultiplied Alpha vs Straight Alpha:错了会有黑边
- 写 VBO 用 `glBufferSubData` 比 `glBufferData` 快(已分配空间)

## 延伸阅读
- [LearnOpenGL — Textures](https://learnopengl.com/Getting-started/Textures)
- [The Cherno — Hazel Engine Sprite Batcher](https://www.youtube.com/watch?v=biGF6oLxgtQ) — 视频教程
- [Unity SRP Batcher 文档](https://docs.unity3d.com/Manual/SRPBatcher.html) — 工业级批渲染
- [Vampire Survivors GDC Talk](https://www.youtube.com/watch?v=H2eoB6V_5LU) — 数千实体怎么 60fps

## 下一章预告
**Ch 05 — 2D 动画 + 粒子 + Tilemap**:有了批渲染,我们就能在屏幕上播放精灵动画、发射粒子、画瓦片地图,做出真正像 2D 游戏的画面。
