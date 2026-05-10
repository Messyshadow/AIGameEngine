# Chapter 09 — Skybox + IBL + Tonemap

> **本章定位**:让 3D 场景有"环境感",画面有"电影感"。

---

## 本章目标
1. **Cubemap**:6 面贴图组成的天空盒
2. **HDR 环境贴图加载**(.hdr 格式)
3. **IBL(Image-Based Lighting)**:用环境贴图当光源
   - **Diffuse Irradiance Map**:卷积出的低频环境漫反射
   - **Prefiltered Environment Map**:不同粗糙度的高光环境
   - **BRDF LUT**:预积分的菲涅尔查找表
4. **Tonemapping**:HDR → SDR 的"色调映射"(ACES Filmic 业界标准)
5. **Gamma 校正**完整管线(sRGB framebuffer)
6. Demo:同一组金属球,在沙漠/雪山/工作室 3 个 HDR 环境下切换

## 跑出来的 Demo
`build/Release/chapter_09_demo.exe` — 5 个金属粗糙度从 0→1 的球,在 3 个 HDR 环境间循环切换(数字键 1/2/3 切换)。金属球能看到环境反射,粗糙球反射模糊。开/关 ACES tonemap 对比。

## 学到的前沿技术
- **Cubemap 采样**:为什么用方向向量索引,不是 UV
- **HDR 文件格式**:RGBE 编码,Radiance .hdr / OpenEXR .exr
- **球面卷积**:对环境贴图做积分得到 irradiance(预计算)
- **重要性采样**:GGX 分布的预过滤
- **Split-Sum Approximation**(Epic UE4 经典)
- **ACES Filmic Tonemap**:从 5 万 nit HDR 压到 100 nit SDR 的工业标准曲线

## 背景知识

只有方向光 + 点光的场景看起来"漂浮在虚空"。**IBL 解决"环境光"问题** — 让金属球反射真实天空,让暗处也有微弱反射光。这是 PBR 看起来真实的关键。

ACES(Academy Color Encoding System)是好莱坞数字电影色彩标准。游戏行业从 2015 年起广泛采用。Tonemap 之前画面是"窗户外的真实亮度",之后是"屏幕能显示的亮度",这个曲线决定了"是否电影感"。

## 架构设计

```
预计算管线(运行一次):
 [.hdr 加载] → [Equirectangular 转 Cubemap] → [卷积 Irradiance Map]
                                            → [预过滤多 mipmap Specular Map]
                                            → [BRDF LUT(2D)]

运行时:
 standard.fs 里:
    vec3 ambient = (kD * irradiance * albedo + specular) * ao;
    其中 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
    最终: color = (Lo + ambient);
    color = ACESFitted(color);
    output = pow(color, 1/2.2);  // gamma
```

## 涉及源文件
- `engine/render/Skybox.h/cpp`
- `engine/render/IBL.h/cpp`(包含 3 个预计算)
- `engine/render/Tonemap.h`
- `data/shaders/skybox.vs/fs`
- `data/shaders/ibl_*.vs/fs`(equirect_to_cube / irradiance_conv / prefilter / brdf_lut)
- `data/textures/hdri/desert.hdr` 等

## ACES 关键代码
```glsl
// ACES Filmic Tone Mapping (Krzysztof Narkowicz 简化版)
vec3 ACESFitted(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}
```

## 课后练习
1. 加 BRDF LUT 的实时预览(画到屏幕一角)
2. 实现 Reinhard / Uncharted 2 / Filmic 多种 Tonemap 切换
3. 加"曝光控制"(模拟相机,让画面整体变亮/变暗)

## 常见坑
- HDR 加载用 stb_image 的 `stbi_loadf`,数据是 float32
- 预过滤 cubemap 要逐 mipmap level 渲染,roughness = mip / maxMip
- BRDF LUT 是 2D 不是 cubemap,xy = (NdotV, roughness)
- Gamma 校正只做一次(framebuffer sRGB 自动 + 手动 pow 二选一)

## 延伸阅读
- [Real Shading in UE4 (Karis 2013)](https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf) — Split-Sum 提出
- [LearnOpenGL — IBL](https://learnopengl.com/PBR/IBL/Diffuse-irradiance) — 完整教程
- [HDR & ACES — Hable Blog](http://filmicworlds.com/blog/filmic-tonemapping-operators/)
- [Polyhaven HDRI](https://polyhaven.com/hdris) — 免费高质量 HDR 环境

## 下一章预告
**Ch 10 — 骨骼动画 + 状态机 + 混合树**:让 3D 角色"动起来",真正像游戏角色而非雕塑。
