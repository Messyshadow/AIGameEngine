# Chapter 08 — PBR 材质 + 光照

> **本章定位**:让 3D 物体"看起来真实"。

---

## 本章目标
1. **Cook-Torrance BRDF**:工业标准 PBR 数学模型
2. **金属-粗糙度工作流**(Disney/Unity 标准)
3. 多种贴图:Albedo / Normal / Metallic / Roughness / AO / Emissive
4. 多光源支持:**Directional + Point + Spot**
5. Demo:5 个材质球(金属、木头、塑料、砖、皮革)在方向光 + 2 个点光下

## 跑出来的 Demo
`build/Release/chapter_08_demo.exe` — 5 个 PBR 材质球排成一排,1 个方向光从右上 45° 照下,2 个点光绕场景旋转。可以看到金属球反射环境、木头有粗糙感、皮革有微高光。

## 学到的前沿技术
- **Cook-Torrance BRDF**:f = D × G × F / (4 × NdotV × NdotL)
  - **D 项**(分布):GGX(Trowbridge-Reitz)— 业界主流
  - **G 项**(几何):Smith Schlick-GGX
  - **F 项**(菲涅尔):Schlick 近似
- **金属-粗糙度工作流**:Albedo 在金属和非金属下的物理含义不同
- **法线贴图采样**:切线空间 vs 世界空间
- **能量守恒**:diffuse + specular ≤ 1
- **Gamma 校正**:sRGB ↔ Linear 的转换

## 背景知识

PBR 是 2014 年迪士尼论文之后逐步统一的标准。**为什么火?**——美术不用为不同光照重画一套贴图,一套贴图通用所有场景。Unity HDRP / Unreal 的 Lit Shader / Godot 4 SpatialMaterial 都是 PBR。

Cook-Torrance 是 1982 年的论文,但直到 GPU 算力够了才普及。关键洞察:**真实材质的高光不是"完美镜面",而是无数微表面反射的统计**。

## 架构设计

```
Material(PBR)
 ├─ albedoTex / albedoColor
 ├─ normalTex
 ├─ metallicTex / metallic (0 = 介电, 1 = 金属)
 ├─ roughnessTex / roughness (0 = 镜面, 1 = 漫反)
 ├─ aoTex
 ├─ emissiveTex / emissiveColor
 └─ shader: standard.vs/fs

Light
 ├─ Directional (dir, color, intensity)
 ├─ Point (pos, range, color)
 └─ Spot (pos, dir, innerCone, outerCone)

LightingSystem
 └─ 把场景中所有 Light 打包传给 shader
   (Forward Renderer 单 pass 限制 N 个光源)
```

## standard.fs 核心代码

```glsl
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// 主光照:
vec3 N = normalize(v_Normal);
vec3 V = normalize(u_CameraPos - v_WorldPos);
vec3 F0 = mix(vec3(0.04), albedo, metallic);
vec3 Lo = vec3(0.0);
for (int i = 0; i < numLights; ++i) {
    vec3 L = normalize(lights[i].pos - v_WorldPos);
    vec3 H = normalize(V + L);
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySchlickGGX(max(dot(N, V), 0.0), roughness)
            * GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metallic);
    vec3 spec = (NDF * G * F) / (4.0 * max(dot(N,V),0.0) * max(dot(N,L),0.0) + 0.001);
    Lo += (kD * albedo / PI + spec) * lights[i].color * max(dot(N, L), 0.0);
}
```

## 涉及源文件
- `engine/render/Material.h/cpp`
- `engine/render/Light.h/cpp`
- `data/shaders/standard.vs/fs`(PBR 标准)
- `data/textures/pbr_balls/` — 5 套 PBR 贴图(从 mini_game_engine 复制)

## 课后练习
1. 实现 ParallaxOcclusion Mapping(视差贴图)
2. 加 SSS(次表面散射)简化版,模拟皮肤
3. 让 light intensity 随距离平方衰减

## 常见坑
- Gamma:贴图加载时要 sRGB → Linear,输出时要 Linear → sRGB(Ch 09 详细)
- 切线空间法线:必须传 tangent + bitangent
- 金属度通常是二值(0 或 1),不要做 0.5 这种(违反物理)
- 性能:每像素跑 4 个光源就开始吃显卡,Forward+ 是 Ch 12 进阶

## 延伸阅读
- [Disney's Principled BRDF](https://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf) — 圣经论文
- [LearnOpenGL — PBR](https://learnopengl.com/PBR/Theory) — 最易懂入门
- [Filament PBR Documentation](https://google.github.io/filament/Filament.md.html) — Google 移动端引擎,数学详尽
- [Real Shading in Unreal Engine 4](https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf)

## 下一章预告
**Ch 09 — Skybox + IBL + Tonemap**:加上天空盒和环境光照,材质球终于不再"漂浮在虚空"。
