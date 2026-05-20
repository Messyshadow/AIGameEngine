# Chapter 09 — Skybox + IBL + Tonemap

> **本章定位**:给 3D 世界一片"天空",让金属反射真实环境。场景不再"漂浮在虚空"。

---

## 一、本章意义

Ch 08 的材质球只被方向光/点光照亮 —— 右上角那些粗糙金属球**发黑**,因为现实里金属之所以好看,是因为它**反射周围环境**(天空、墙、地面)。没有环境,金属就是黑的。

本章加两样东西:
1. **Skybox(天空盒)** —— 场景背景是真实拍摄的 HDR 全景图
2. **IBL(Image-Based Lighting,基于图像的光照)** —— 用这张环境图当光源,金属球反射天空,所有物体都有自然的环境光

学完本章,AIForge 的 3D 画面达到**产品级真实感** —— 这是 Sketchfab、ArtStation 上模型展示的标准做法。

---

## 二、本章目标

| 目标 | 验收 |
|---|---|
| 加载 **HDR 全景图**(.hdr 浮点纹理) | RGB16F + mipmap |
| **Skybox** 渲染 | 背景是真实天空,跟随相机 |
| **IBL 镜面反射** | 金属球反射天空,粗糙度越大反射越模糊 |
| **IBL 漫反射环境光** | 暗处也有来自天空的柔和补光 |
| **HDR Tonemap + Gamma** | 高动态范围正确压到屏幕 |
| 切换 3 张 HDRI | 数字键 1/2/3,看不同环境下材质的变化 |

---

## 三、为什么需要 HDR(高动态范围)

普通图片每通道 8 位(0-255),表示不了真实世界的亮度跨度 —— 太阳比阴影亮**几万倍**。

**HDR(.hdr / .exr)** 用浮点存储,亮度可以远超 1.0。这对光照至关重要:
- 用 HDR 当光源,亮的地方(太阳)真的很亮 → 金属高光才有"刺眼"的真实感
- 最后用 **Tonemap** 把这个超大范围压回屏幕能显示的 [0,1]

本章的 HDR 来自 [Polyhaven](https://polyhaven.com)(CC0 免费),已自动下载到 `data/textures/hdri/`。

---

## 四、Equirectangular(等距柱状)全景图

HDR 全景图是 **2:1 的"世界地图"投影** —— 就像把整个球形天空摊平成一张矩形图(经度→横轴,纬度→纵轴)。

**方向 ↔ UV 的转换**(本章核心数学):
```glsl
const vec2 invAtan = vec2(0.15915494, 0.31830989);  // (1/2pi, 1/pi)
vec2 dirToUV(vec3 d) {
    vec2 uv = vec2(atan(d.z, d.x), asin(clamp(d.y, -1.0, 1.0)));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}
```
给一个方向向量(看天空的方向 / 反射方向 / 法线方向),算出在全景图上采样的 UV。**天空盒、IBL 反射、IBL 环境光,全靠这一个函数。**

---

## 五、Skybox 怎么画 —— 两个技巧

```glsl
// 顶点着色器
gl_Position = (proj * mat3ToMat4(view) * vec4(a_Pos, 1.0)).xyww;
```

**技巧 1:去掉视图矩阵的平移**(只保留旋转)→ 天空盒永远以相机为中心,你走多远天空都"够不着"(无限远)。

**技巧 2:`.xyww`** → 透视除法后深度 = w/w = 1.0(远平面)。配合 `glDepthFunc(GL_LEQUAL)`,天空盒只画在"没有任何物体"的背景像素上,被物体遮住的地方自动不画。

---

## 六、IBL —— 本章的灵魂

IBL 把"环境图"当成一个**360° 的光源**。物体表面收到的环境光分两部分:

### 漫反射环境光(Diffuse IBL)
表面朝哪个方向(法线 N),就接收那个方向**附近一大片**天空的平均亮度。
```glsl
// 用高 mip(模糊版)采样 = 廉价的"辐照度"近似
vec3 irradiance = textureLod(u_Env, dirToUV(N), maxMip - 2.0).rgb;
vec3 diffuseIBL = irradiance * albedo;
```

### 镜面反射环境光(Specular IBL)
光滑表面像镜子,反射 `R = reflect(-V, N)` 方向的天空;粗糙表面反射模糊。
```glsl
vec3 R = reflect(-V, N);
// 粗糙度 -> mip 层级:越粗糙采样越模糊的 mip
vec3 prefiltered = textureLod(u_Env, dirToUV(R), roughness * maxMip).rgb;
vec3 specularIBL = prefiltered * fresnel;
```

**关键技巧**:用纹理的 **mipmap** 模拟"粗糙度模糊"。光滑(roughness=0)采样 mip 0(清晰),粗糙(roughness=1)采样最高 mip(最模糊)。一行 `textureLod` 搞定,这就是本章"省下完整 IBL 管线"的窍门。

---

## 七、严谨说明 —— 本章是"简化版 IBL"

工业级 IBL(Unreal/Unity)用 **Split-Sum 近似**(Karis 2013),完整管线是:
1. equirect → cubemap(渲染到 6 个面)
2. **辐照度卷积**(Irradiance Map):对 cubemap 半球积分,得到精确漫反射
3. **预过滤环境图**(Prefiltered Map):按 GGX 重要性采样,每个 mip 对应一个粗糙度
4. **BRDF LUT**:预积分菲涅尔 + 几何项的 2D 查找表
5. 最终:`specular = prefiltered * (F * brdf.x + brdf.y)`

本章用 **equirect 直采 + mipmap 模糊** 替代了步骤 1-4。**视觉上 80% 接近,代码量 1/4,出错面极小。** 完整 Split-Sum 版作为进阶练习(见课后),也是 Ch 12 的潜在升级。

为什么这么选?因为 AIForge 是教学引擎 —— 先用简单方法把"环境光照"的概念讲透、跑通,再谈精确。完整 IBL 一旦有个 cubemap 朝向写反,整章黑屏且极难调试。

---

## 八、新增文件

| 文件 | 内容 |
|---|---|
| `engine/render/Texture.h/cpp` | 加 `CreateFromHDR`(RGB16F + mipmap)+ `GetMaxMipLevel` |
| `engine/render/Skybox.h/cpp` | 天空盒(equirect 直采) |
| `examples/09_*/main.cpp` | demo:PBR 球阵 + 天空盒 + IBL,可切 3 张 HDRI |
| `data/textures/hdri/*.hdr` | 3 张 Polyhaven CC0 HDRI(已登记 CREDITS.md) |

---

## 九、Demo 体验

`chapter_09_demo.exe`:
- 背景是**真实 HDR 天空**(默认威尼斯日落)
- **7×7 PBR 球阵**沐浴在环境光里:
  - **金属球反射天空** —— 能看到天空的颜色映在球面上
  - **粗糙金属**反射模糊(磨砂金属感),**光滑金属**反射清晰(镜面)
  - 非金属球有来自天空的柔和漫反射补光
- 对比 Ch 08:右上角的粗糙金属球**不再发黑**,有了环境反射

**控制**:
| 操作 | 效果 |
|---|---|
| 鼠标右键拖动 | 绕场景旋转 |
| 滚轮 | 缩放 |
| **1 / 2 / 3** | 切换 HDRI(日落 / 阴天户外 / 室内影棚)|
| ESC | 退出 |

**最有教育意义**:按 1/2/3 切换环境 —— 同一组球,在不同环境下质感完全不同。日落环境下金属泛橙,影棚环境下金属泛白。这就是 IBL:**环境决定材质表现**。

---

## 十、课后练习

1. 实现完整 Split-Sum IBL(equirect→cubemap + 辐照度卷积 + 预过滤 + BRDF LUT)
2. 加 BRDF LUT,让镜面反射更准(边缘菲涅尔正确)
3. 天空盒加"曝光"控制(模拟相机,整体调亮/暗)
4. 让物体能被天空"间接照亮"形成柔和阴影(SSAO,Ch 12)

---

## 十一、常见坑

- **HDR 必须用浮点纹理**(GL_RGB16F),用 RGBA8 会把 >1 的光截断
- **方向归一化** —— dirToUV 前 `normalize`,否则 UV 错
- **mipmap 必须生成**(`glGenerateMipmap`),否则 roughness 模糊无效
- **天空盒深度** —— 忘了 `.xyww` + `GL_LEQUAL`,天空会盖住物体或被裁掉
- **极点接缝** —— equirect 在正上/正下方有轻微挤压,demo 级可接受
- **gamma 只做一次** —— 天空盒和物体都 tonemap+gamma,别重复

---

## 十二、延伸阅读

- [LearnOpenGL — IBL Diffuse Irradiance](https://learnopengl.com/PBR/IBL/Diffuse-irradiance)
- [LearnOpenGL — IBL Specular](https://learnopengl.com/PBR/IBL/Specular-IBL)
- [Real Shading in UE4 (Karis 2013, Split-Sum)](https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf)
- [Polyhaven HDRI 库(CC0)](https://polyhaven.com/hdris)

---

## 十三、下一章预告

**Ch 10 — 骨骼动画 + 状态机 + 混合树**:3D 篇章前半到此。下一章让 3D 角色"动起来" —— Assimp 加载带骨骼的模型,蒙皮、动画播放、idle/walk/run 状态切换。**第一个会动的 3D 角色。**
