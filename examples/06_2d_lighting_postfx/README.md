# Chapter 06 — 2D 光照 + 后处理(Bloom)

> **本章定位**:让 2D 画面从"独立小作品"跨进"商业品质"。同一组美术,加光照 + 后处理,质感能翻一倍。

---

## 一、本章为什么是质变

Ch 04-05 我们能画 sprite、动画、粒子、瓦片地图。但画面是**全亮的、平的、没有氛围**。

对比一下:
- 没光照:*一张白纸上贴贴纸*
- 有光照 + Bloom:*Hyper Light Drifter / Dead Cells / CrossCode 的电影感*

**关键事实**:玩家对"画面质感"的感知,**70% 来自光照和后处理**,只有 30% 来自美术资源本身。同一张机甲贴图,放在全亮场景里平平无奇,放在黑暗中被一束暖光打亮——立刻有了戏剧性。

学完本章,AIForge 的 2D 表现力达到 **Steam 在售独立游戏** 的水准。

---

## 二、本章目标

| 目标 | 验收 |
|---|---|
| **Framebuffer(FBO)**:渲染到纹理而非屏幕 | 多 pass 渲染的基础 |
| **2D 动态光照**:点光源 + 环境光 | 黑暗场景里有光晕 |
| **光照合成**:`最终 = 场景色 × 光照` | 暗处变暗,亮处变亮 |
| **Bloom**:高亮像素溢光 | 粒子/光源发光 |
| **Tonemap + Gamma**:HDR→SDR | 颜色不过曝 |
| **L / B 开关** | 实时对比"开光照/关光照""开 Bloom/关 Bloom" |

---

## 三、核心概念 — 这章你会真正理解

### 3.1 Framebuffer(FBO)= 离屏画布

之前我们的渲染都直接画到**屏幕**。但后处理需要"先画到一张纹理上,再对这张纹理做特效"。

```
传统:  绘制 ───────────────────────────→ 屏幕

后处理: 绘制 → [FBO 纹理] → 特效处理 → 屏幕
              ↑ 这就是 Framebuffer
```

FBO 是现代渲染的基石:阴影贴图、延迟渲染、后处理、镜面反射……全靠它。

### 3.2 多 Pass 渲染

Ch 06 一帧要画 **4 趟**:

```
Pass 1  画场景  → sceneFBO     (瓦片+角色+粒子,正常 alpha 混合)
Pass 2  画光照  → lightFBO     (清成环境光,再 additive 叠加每个光源)
Pass 3  合成    → compositeFBO (sceneFBO × lightFBO)
Pass 4  Bloom + 输出 → 屏幕     (提取高亮→模糊→叠加→tonemap)
```

### 3.3 2D 光照的本质 —— 一次乘法

```
最终颜色 = 场景颜色 × 光照值

光照值 = 环境光(全局基础亮度) + Σ 每个光源的贡献

举例:
  环境光 = 0.2(很暗)
  某像素没被任何光照到 → 光照值 = 0.2 → 最终 = 场景 × 0.2(暗)
  某像素在火把范围内    → 光照值 = 1.0 → 最终 = 场景 × 1.0(亮)
```

**怎么实现光源?** 不用复杂数学 —— 每个光源就是一张**径向渐变贴图**(中心白、边缘黑),用 **加法混合(Additive Blend)** 叠到 lightFBO 上。这就是为什么本章给 SpriteBatcher 加了 `BlendMode::Additive`。

### 3.4 加法混合 vs 普通混合

| 混合模式 | 公式 | 用途 |
|---|---|---|
| **Alpha**(普通) | `结果 = 新色×α + 旧色×(1-α)` | sprite、UI、半透明 |
| **Additive**(加法) | `结果 = 新色×α + 旧色` | **光源、火焰、发光粒子、激光** |

加法混合的特点:**越叠越亮**。两个光源重叠处会更亮——这正是真实光照的行为。

### 3.5 Bloom — 高光溢出

现实中,看强光会"晃眼",光会"溢出"到周围。Bloom 模拟这个:

```
1. 提取高亮:  亮度 > 阈值的像素才保留,其余变黑
2. 高斯模糊:  把这些高亮像素"晕开"
3. 叠加回去:  原图 + 模糊后的高亮 = 溢光效果
```

**可分离高斯模糊**(Separable Gaussian)是关键优化:二维模糊 = 先横向模糊 + 再纵向模糊,把 O(n²) 降到 O(2n)。本章用 **ping-pong**(两个 FBO 来回画)实现。

### 3.6 Tonemap + Gamma

光照累加后颜色可能 >1(过曝)。**Tonemap** 把 HDR 范围压回 [0,1]:
```glsl
c = c / (c + 1.0);   // Reinhard,最简单的 tonemap
c = pow(c, 1/2.2);   // Gamma 校正(线性空间 → sRGB 显示)
```

---

## 四、AIForge Ch 06 架构

```
                  ┌─────────────────┐
   场景绘制 ──────→│  sceneFBO       │──┐
   (Alpha 混合)    └─────────────────┘  │
                                        ├──→ composite = scene × light
   光照绘制 ──────→┌─────────────────┐  │         │
   (Additive 混合) │  lightFBO       │──┘         ▼
   清成环境光      └─────────────────┘     ┌──────────────┐
                                          │ 提取高亮      │
                                          │   ↓          │
                                          │ 高斯模糊×N    │  Bloom
                                          │   ↓          │
                                          │ composite+bloom
                                          │   ↓ tonemap   │
                                          └──────┬───────┘
                                                 ▼
                                               屏幕
```

---

## 五、新增 / 修改的文件

| 文件 | 变化 |
|---|---|
| `engine/render/Framebuffer.h/cpp` | 新建:FBO 离屏渲染目标 |
| `engine/render/PostProcess.h/cpp` | 新建:全屏四边形 + Bloom + 合成,4 个后处理 shader |
| `engine/render/SpriteBatcher.h/cpp` | 修改:`Begin` 加 `BlendMode`(Alpha / Additive) |
| `engine/render/Texture.h/cpp` | 修改:`CreateFromFile` 优先用 .exe 目录锚点(修 Ch 05 加载问题) |
| `examples/06_*/main.cpp` | 新建:黑暗场景 + 动态光 + Bloom demo |

---

## 六、关键代码

### SpriteBatcher 加法混合(画光源)
```cpp
// 普通画场景
batcher.Begin(camera, sceneTex, SpriteBatcher::BlendMode::Alpha);
// ...
batcher.End();

// 加法画光源 —— 越叠越亮
batcher.Begin(camera, radialLightTex, SpriteBatcher::BlendMode::Additive);
for (auto& light : lights)
    batcher.Submit(light.pos, {light.radius*2, light.radius*2}, light.color);
batcher.End();
```

### 后处理合成 shader(scene × light)
```glsl
vec3 scene = texture(u_Scene, v_UV).rgb;
vec3 light = texture(u_Light, v_UV).rgb;
frag = vec4(scene * light, 1.0);
```

### 可分离高斯模糊(ping-pong)
```cpp
bool horizontal = true;
for (int i = 0; i < blurIterations * 2; ++i) {
    blurFBO[horizontal ? 0 : 1].Bind();
    blurShader.SetVec2("u_Direction",
        horizontal ? Vec2{texelW,0} : Vec2{0,texelH});
    // 第一次读 brightFBO,之后读上一张 blurFBO
    DrawFullscreenQuad();
    horizontal = !horizontal;
}
```

---

## 七、Demo 体验

`chapter_06_demo.exe`:
- 一个**黑暗的瓦片世界**(环境光很低,几乎看不清)
- 一个机甲角色(robot3),**自带一束暖光火把**
- 场景里散布几盏**静态彩色灯**(青/品红/橙)
- 角色周围飘**发光的橙色火星粒子**
- 整个画面有 **Bloom 溢光**

**控制**:
| 按键 | 效果 |
|---|---|
| WASD | 移动角色(火把光跟随) |
| **L** | 开 / 关光照 —— 对比"全亮平面" vs "黑暗+光" |
| **B** | 开 / 关 Bloom —— 对比有无溢光 |
| ESC | 退出 |

**最有教育意义的操作**:按 **L** 来回切。关掉光照时,你看到 Ch 05 那种"全亮平面";打开时,同样的美术立刻有了氛围和戏剧性。**这一下就是本章的全部意义。**

---

## 八、课后练习

1. 给光源加"呼吸"效果(半径/亮度随 sin 波动,像火把摇曳)
2. 实现 **2D 阴影投射**:从光源发射射线,被瓦片挡住的地方变暗(进阶,有难度)
3. 把 FBO 改成 `GL_RGBA16F`(HDR),光照能超过 1.0,Bloom 更有层次
4. 加一个 **暗角(Vignette)** 后处理:屏幕边缘渐暗

---

## 九、常见坑

- **FBO 没清屏**:每帧 `Bind()` 后要 `glClear`,否则上一帧残留
- **viewport 不匹配**:FBO 和屏幕尺寸不同,`Bind()` 要重设 viewport(我们封装好了)
- **采样与写入同一纹理**:Bloom ping-pong 必须用两个 FBO 交替,不能边读边写
- **混合状态泄漏**:后处理 pass 要 `glDisable(GL_BLEND)`,结束后恢复
- **RGBA8 截断**:8 位 FBO 颜色钳在 [0,1],强光 HDR 需要 RGBA16F(Ch 12)

---

## 十、延伸阅读

- [LearnOpenGL — Framebuffers](https://learnopengl.com/Advanced-OpenGL/Framebuffers)
- [LearnOpenGL — Bloom](https://learnopengl.com/Advanced-Lighting/Bloom)
- [2D Lighting in Unity URP](https://blog.unity.com/games/the-2d-lighting-system-in-unity)
- [GPU Gems — Real-Time Glow](https://developer.nvidia.com/gpugems/gpugems/part-iv-image-processing/chapter-21-real-time-glow)
- [Hyper Light Drifter 光影解析](https://www.gamedeveloper.com/art/hyper-light-drifter-s-light-and-shadow)

---

## 十一、下一章预告

**Ch 07 — 3D Mesh + Camera3D**:2D 篇章到此完结。下一章进入 3D 世界 —— MVP 矩阵、透视投影、第一个旋转立方体、自由飞行摄像机。
