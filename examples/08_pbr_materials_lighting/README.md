# Chapter 08 — PBR 材质 + 光照

> **本章定位**:让 3D 物体"有材质"。从 Ch 07 的纯色方块,跨进"金属/陶瓷/塑料一眼能分辨"的次世代画质。

---

## 一、本章意义

Ch 07 的立方体是纯色的 —— 它们看起来像"染了色的塑料块",没有质感。真实世界里,金属、木头、陶瓷、橡胶在光下的表现**完全不同**:
- 金属:几乎没有漫反射,高光锐利、带颜色
- 陶瓷:柔和漫反射 + 清晰高光
- 橡胶:漫反射为主,高光弥散

**PBR(Physically Based Rendering,基于物理的渲染)** 用一套统一的数学,让同一个着色器能表现所有这些材质。这是 2014 年后所有主流引擎(Unreal / Unity HDRP / Godot 4 / Frostbite)的标准。

学完本章,AIForge 的 3D 画面进入**次世代**。

---

## 二、本章目标

| 目标 | 验收 |
|---|---|
| 理解 PBR 相对传统 Phong 的本质区别 | 能说清"为什么 PBR 更真实" |
| 实现 Cook-Torrance BRDF(D/G/F 三项) | GGX + Smith + Schlick |
| 金属-粗糙度工作流 | metallic / roughness 两个参数驱动一切 |
| 多光源:方向光 + 点光 | 点光带距离衰减 |
| HDR tonemap + Gamma 校正 | 颜色不过曝 |
| Demo:7x7 材质球阵 | 金属度纵轴 x 粗糙度横轴 |

---

## 三、PBR vs 传统 Phong —— 本质区别

### 老办法:Phong / Blinn-Phong(1975)
```
高光 = pow(dot(reflect, view), shininess)
```
问题:纯靠美术调参数。`shininess` 是个魔法数字,没有物理含义。换个光照环境,美术得重调一套。金属和塑料要写两套逻辑。

### PBR:基于物理(2014~)
PBR 的核心信条:
1. 能量守恒 —— 反射出去的光不可能比射入的多
2. 微表面理论 —— 再光滑的表面,放大看都是无数微小镜面;"粗糙度"就是这些微镜面朝向的混乱程度
3. 菲涅尔效应 —— 任何材质,在掠射角(几乎平视)都会变成镜子
4. 统一参数 —— 所有材质用同一组物理参数(albedo / metallic / roughness)描述

结果:一套着色器,一套贴图,在任何光照下都正确。

---

## 四、Cook-Torrance BRDF —— 本章的数学核心

BRDF(双向反射分布函数)回答:"光从 L 方向来,有多少反射到 V 方向?"

Cook-Torrance 把反射分成漫反射 + 镜面反射:
```
出射光 = (kD * 漫反射 + 镜面反射) * 辐射度 * NdotL
镜面反射 = (D * G * F) / (4 * NdotV * NdotL)
```

### D 项 —— 法线分布(GGX / Trowbridge-Reitz)
多少微表面正好朝向能产生高光的方向。
```glsl
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a  = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float d = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}
```
粗糙度小 -> 高光小而锐;粗糙度大 -> 高光大而散。

### G 项 —— 几何遮蔽(Smith + Schlick-GGX)
微表面互相遮挡,损失多少光。
```glsl
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}
```

### F 项 —— 菲涅尔(Schlick 近似)
掠射角反射增强。
```glsl
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0-cosTheta,0.0,1.0), 5.0);
}
```
`F0` = 垂直入射基础反射率:非金属 ≈ 0.04,金属 = 它的 albedo。

---

## 五、金属-粗糙度工作流

PBR 用两个标量描述绝大多数材质:

| 参数 | 含义 | 0 | 1 |
|---|---|---|---|
| metallic | 是否金属 | 介电体(塑料/木/陶瓷) | 纯金属 |
| roughness | 表面有多粗糙 | 镜面光滑 | 完全漫反射 |

外加 albedo(基础色):非金属 = 漫反射色,金属 = 高光色。

关键代码技巧:
```glsl
vec3 F0 = mix(vec3(0.04), albedo, metallic);   // 金属 F0 = albedo
vec3 kD = (1.0 - F) * (1.0 - metallic);        // 金属没有漫反射
```

---

## 六、多光源 + 衰减

| 光源 | 特征 |
|---|---|
| 方向光(Directional) | 平行光,无衰减 —— 太阳 |
| 点光(Point) | 距离平方反比衰减 —— 灯泡/火把 |

点光衰减:`辐射度 = 光色 / 距离²`。

---

## 七、HDR Tonemap + Gamma

```glsl
color = color / (color + vec3(1.0));   // Reinhard tonemap
color = pow(color, vec3(1.0 / 2.2));   // Gamma 校正
```
没有这两步,亮处"死白"一片。

---

## 八、新增文件

| 文件 | 内容 |
|---|---|
| `engine/render/Material.h` | PBR 材质结构(albedo / metallic / roughness / ao) |
| `engine/render/Light.h` | 光源结构(DirectionalLight / PointLight) |
| `examples/08_*/main.cpp` | demo:7x7 材质球阵 + Cook-Torrance shader |

> Material / Light 是纯数据结构(header-only)。Cook-Torrance shader 暂放 demo 里,Ch 09 会升级成引擎级材质系统。

---

## 九、Demo 体验

`chapter_08_demo.exe`:
- 7x7 = 49 个球排成网格
  - 纵轴:金属度 metallic 从 0(下)到 1(上)
  - 横轴:粗糙度 roughness 从 0.05(左)到 1.0(右)
- 一束方向光 + 4 个绕场景旋转的点光(暖橙/冷青/品红/绿)
- 你能直观看到:
  - 左下:光滑非金属 -> 清晰小高光
  - 右下:粗糙非金属 -> 高光弥散
  - 左上:光滑金属 -> 锐利彩色反射
  - 右上:粗糙金属 -> 金属磨砂感

控制:
| 操作 | 效果 |
|---|---|
| 鼠标右键拖动 | 绕球阵旋转视角 |
| 滚轮 | 拉近/拉远 |
| 空格 | 暂停/继续点光旋转 |
| ESC | 退出 |

---

## 十、课后练习

1. 加法线贴图(给 `Mesh::Vertex` 加切线 tangent)
2. 加聚光灯(Spot Light)
3. 把材质参数改成贴图驱动
4. 实现 emissive 自发光

---

## 十一、常见坑

- Gamma 只做一次,重复 = 颜色发灰
- 金属忘了 `kD *= (1-metallic)` 会发灰
- `4 * NdotV * NdotL` 要加 `+ 0.0001` 防除零
- 点光反平方衰减很猛,强度要给大(本 demo 用 ~300)
- 法线插值后要重新归一化

---

## 十二、延伸阅读

- [LearnOpenGL — PBR Theory](https://learnopengl.com/PBR/Theory)
- [Disney's Principled BRDF (2012)](https://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf)
- [Real Shading in Unreal Engine 4](https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf)
- [Google Filament PBR 文档](https://google.github.io/filament/Filament.md.html)

---

## 十三、下一章预告

**Ch 09 — Skybox + IBL + Tonemap**:加天空盒 + 基于图像的光照(IBL),金属球能反射真实天空,场景不再"漂浮在虚空"。
