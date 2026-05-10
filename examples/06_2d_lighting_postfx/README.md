# Chapter 06 — 2D 光照 + 后效

> **本章定位**:把 2D 提升到"像素艺术大作"质感。

---

## 本章目标
1. **2D Normal Map**:给 sprite 加法线贴图,模拟伪 3D 光照
2. **2D Shadow Casting**:点光源被遮挡物阻挡(Ray Marching 2D 版)
3. 后处理:**Bloom + Color Grading + Vignette + CRT 滤镜**(可选)
4. 像素艺术保真采样(point 采样而非线性,避免模糊)
5. Demo:洞穴探险风格场景,角色举火把,墙壁有阴影

## 跑出来的 Demo
`build/Release/chapter_06_demo.exe` — 黑暗洞穴 + 像素角色,角色身上挂一个 360° 点光源,墙壁瓦片是遮挡物,呈现戏剧性光影。

## 学到的前沿技术
- **2D 法线贴图**:让平面 sprite 在不同光照角度有立体感(*Hyper Light Drifter* / *Dead Cells* 的秘密)
- **Light Volume / Light Mask**:光照单独渲染到 RT,叠加到主画面
- **Ray-Cast Shadow**:从光源发射 N 条射线找遮挡物的算法
- **Multi-Render-Target / Framebuffer Object**(FBO):后处理基础
- **Pixel-Perfect Filtering**:像素艺术不能用线性插值

## 背景知识

为什么 2D 也要 normal map?——因为玩家眼睛对"光照"极其敏感。同样的像素角色,加 normal + 动态光,质感能从"独立小作品"升到"商业作品"。代表:*Don't Starve* / *Hollow Knight* / *CrossCode*。

后处理(Bloom / Vignette / Color Grading)是"游戏感"的最大单一来源。Vampire Survivors 的整个画面氛围 80% 来自后处理。

## 架构设计

```
2D 光照管线:
 [SpriteBatcher 出 albedo+normal RT]
       ↓
 [LightAccumulator 用 light_volumes 算 light RT]
       ↓
 [Compose 把 albedo × light + 后处理]
       ↓
 [屏幕]

Light2D
 ├─ pos / radius / color / intensity
 ├─ castShadow: bool
 └─ shadow: 用 ray cast 在 occluder 数据中算

PostProcessStack
 ├─ Bloom (高斯模糊 + 阈值)
 ├─ Vignette (径向暗角)
 ├─ ColorGrading (LUT 3D)
 └─ FXAA(可选)
```

## 涉及源文件
- `engine/render/Light2D.h/cpp`
- `engine/render/PostProcess.h/cpp`(2D 版,3D 版在 Ch 12 扩展)
- `engine/render/Framebuffer.h/cpp`(FBO 抽象)
- `data/shaders/light2d.vs/fs`
- `data/shaders/post_bloom.vs/fs`

## 课后练习
1. 加方向光(2D 版)模拟早晨/黄昏
2. 实现"光晕"sprite(光源中心的高光)
3. CRT 滤镜:扫描线 + 屏幕弧度

## 常见坑
- Normal map 需要"切线空间"概念,2D 简化为"屏幕空间"即可
- FBO 大小要跟主窗口同步(resize 时重建)
- 后处理 Pass 多了会拖累低端 GPU,提供"质量等级"开关

## 延伸阅读
- [2D Lighting in Unity URP](https://blog.unity.com/games/the-2d-lighting-system-in-unity)
- [Hyper Light Drifter Lighting Breakdown](https://www.gamedeveloper.com/art/hyper-light-drifter-s-light-and-shadow)
- [GPU Gems 1 — Bloom](https://developer.nvidia.com/gpugems/gpugems/part-iv-image-processing/chapter-21-real-time-glow)
- [Catlike Coding — 2D Lights](https://catlikecoding.com/unity/tutorials/) — Unity 视角但概念通用

## 下一章预告
**Ch 07 — 3D Mesh + Camera3D**:从 2D 进入 3D 世界,理解 MVP 矩阵,画第一个旋转的立方体。
