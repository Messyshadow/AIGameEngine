# Chapter 05 — 2D 动画 + 粒子 + Tilemap

> **本章定位**:让画面"活"起来。一个 Vampire Survivors 雏形所需的全部 2D 渲染基建,本章后齐活。

---

## 一、本章为什么是 2D 游戏的"魂"

Ch 04 让你能画 5000 sprite。但所有 sprite 都是同一张图、不动嘴不动脚、世界一片空白 —— **不是游戏**。

Ch 05 解决三个"为什么 2D 游戏才是游戏"的问题:

| 问题 | Ch 04 状态 | Ch 05 解法 |
|---|---|---|
| 角色没有动作 | 一张静态贴图 | **Sprite Sheet 动画** — idle / walk / run 帧切换 |
| 画面没有特效 | 静态 sprite | **粒子系统** — 拖尾 / 爆炸 / 烟雾 / 闪光 |
| 没有世界背景 | 黑屏 + sprite | **Tilemap** — 数据驱动的瓦片地图 |

学完本章你能做:
- 类 **Hades** 的角色动作切换(idle/walk/attack)
- 类 **Vampire Survivors** 的粒子拖尾
- 类 **Stardew Valley** 的瓦片世界

---

## 二、本章目标(具体可验)

| 目标 | 验收 |
|---|---|
| 加载真实 PNG sprite sheet(`robot3_idle.png`) | stb_image 集成 |
| 角色播放 6-frame idle 循环 | 平滑无跳变 |
| WASD 时切换到 walk 动画,松开回 idle | 状态切换无缝 |
| 角色脚下 60 粒/秒尘土粒子 | 颜色 / 大小渐变,生命周期 1.5s |
| 32×24 瓦片地图(草地 / 石头 / 水) | 程序化生成,摄像机平移可见 |
| 整个画面 **单 draw call**(继承 Ch 04 批渲染) | 标题栏证明 |

---

## 三、行业故事 — 这三件事怎么撑起 indie 神作

### 3.1 Sprite Sheet 动画的 30 年传承

```
1990 任天堂 SNES         一帧一帧手绘,放进 tile RAM 切换
2000 GBA  / Flash         同样套路,工业化
2010 Spine / Aseprite     专业工具时代
2020 Hades / Hollow Knight  顶级独立游戏的核心管线
```

**关键洞察**:8 帧画一个 "walk" 周期,玩家眼睛 12 FPS 就能感受到流畅运动。这是大脑视觉补帧的奇迹。

### 3.2 粒子系统是"游戏感"的最大单一来源

| 游戏 | 没粒子时 | 加粒子后 |
|---|---|---|
| Vampire Survivors | 圆形子弹敌人,无趣 | 击中飞溅、击杀爆炸、升级光环 ⭐ |
| Brotato | 同 | 武器特效让 1 分钟感觉 10 分钟 |
| Risk of Rain 2 | 弹幕躲避 | 击杀粒子让"爽快"有数学公式 |

**经验法则**:玩家的"爽快"70% 来自粒子和音效,30% 来自玩法。

### 3.3 Tilemap = "我有一个世界" 的最低成本实现

```
单张 32×32 草地纹理
  × 重复贴 100×100 = 一个开放世界(只占 4 KB 显存)
```

Stardew Valley 整个游戏世界基本就是 tilemap + 几张装饰图。 *Don't Starve* / *Terraria* 同样套路。**Tilemap 是 indie 团队做大世界的钥匙**。

---

## 四、核心技术 — 这一章你会真正理解

### 4.1 Sprite Sheet 数据模型

**一张大图,网格切片**:
```
robot3_idle.png  (1400×1400)
+------+------+------+------+------+------+------+
| f0   | f1   | f2   | f3   | f4   | f5   | f6   |  row 0
+------+------+------+------+------+------+------+
| f7   | f8   | ...                              |  row 1
+------+------+------+------+------+------+------+
   200    200    200    200    200    200    200
```

代码访问:`spriteSheet.GetFrameUV(frameIndex)` -> 返回 `Vec4{u_min, v_min, u_max, v_max}` 给 SpriteBatcher 用。

### 4.2 帧动画播放器

```cpp
class SpriteAnimation {
public:
    std::vector<int> frames;   // 哪些帧组成这个动画
    float            fps = 8;
    bool             loop = true;

    void  Tick(float dt);
    int   GetCurrentFrameIndex() const;
};
```

每帧 Tick:`m_time += dt`,根据 `m_time * fps` 算出当前帧。**关键:用累加器而不是取模时间**,否则帧率不稳时会跳帧。

### 4.3 粒子系统(CPU 版,GPU 版留 Ch 13)

**架构**:
```
ParticleEmitter
 |- 配置:发射形状 / 速率 / 生命 / 颜色渐变 / 大小渐变
 |- 池:预分配 max=10000 的 Particle,避免每次 new
 |- Update(dt):
 |    1. 累计发射时间,够了就 Spawn 一个粒子
 |    2. 遍历活着的粒子,更新 pos / 颜色 / 大小,过期回收

每个 Particle:
   Vec2 pos, vel, accel
   float age, life
   Vec4 colorStart, colorEnd
   Vec2 sizeStart, sizeEnd
```

**性能要点**:**SoA(Structure of Arrays)** 比 **AoS** 缓存友好。本章先用 AoS 求清晰,Ch 13 GPU 粒子改 SoA。

### 4.4 Tilemap 数据结构

**数据**:`std::vector<uint16_t> tiles`(每个 cell 一个瓦片索引)。

**渲染优化 — 视锥裁剪**:
```cpp
// 只画屏幕里能看到的瓦片
int startX = max(0, (cam.x - viewHalfW) / tileSize);
int endX   = min(W-1, (cam.x + viewHalfW) / tileSize);
// 同理 Y
for (y in [startY, endY])
    for (x in [startX, endX])
        batcher.Submit(...)
```

一张 100×100 地图,屏幕里只有 40×24 个瓦片可见 -> 960 次 Submit,几乎免费。

---

## 五、AIForge Ch 05 架构图

```
                     Camera2D (从 Ch 04 继承)
                          |
                          v
                     SpriteBatcher (Ch 04,本章扩展 uvRect 参数)
                     /      |       \
                    /       |        \
              Tilemap    Character    ParticleSystem
              (背景)     (动画+控制)   (拖尾+特效)

每帧:
  1. 主循环 Tick -> 输入处理
  2. Character.Animator.Tick(dt) -> 当前帧
  3. ParticleSystem.Update(dt) -> 移动 / 老化粒子
  4. batcher.Begin
  5. tilemap.Render(batcher)       <- 先画背景
  6. particles.Render(batcher)     <- 中间画粒子
  7. character.Render(batcher)     <- 最后画角色
  8. batcher.End -> 单 draw call
```

---

## 六、新增 / 修改的文件

| 文件 | 变化 |
|---|---|
| `engine/render/Texture.h/cpp` | 修改:加 `CreateFromFile(path)` 用 stb_image PNG 加载 |
| `engine/render/SpriteBatcher.h/cpp` | 修改:`Submit` 加 `uvRect` 参数(默认 `{0,0,1,1}` 兼容 Ch 04) |
| `engine/render/SpriteSheet.h/cpp` | 新建:包装 Texture + cell 网格 |
| `engine/render/SpriteAnimation.h/cpp` | 新建:帧序列 + fps + loop |
| `engine/render/ParticleSystem.h/cpp` | 新建:粒子池 + Emitter |
| `engine/render/Tilemap.h/cpp` | 新建:数据驱动瓦片地图 |
| `examples/05_*/main.cpp` | 新建:demo |
| `data/textures/character/robot3_*.png` | 复制:从用户素材库 |

---

## 七、Sprite Sheet 行业工具(扩展知识)

| 工具 | 用途 | 输出 |
|---|---|---|
| [Aseprite](https://www.aseprite.org/) | 像素艺术 + 动画绘制 | `.aseprite` 项目 -> 导出 `.png` + `.json` |
| [TexturePacker](https://www.codeandweb.com/texturepacker) | sprite atlas 打包 | `.png` + `.atlas` 文件 |
| [Spine](http://esotericsoftware.com/) | 骨骼动画 | `.atlas` + `.json` + `.skel` |
| [DragonBones](https://dragonbones.com/) | 免费骨骼动画 | 类似 Spine |
| [Kenney 素材](https://kenney.nl/) | **免费 CC0 sprite** | 1000+ 素材包,商业可用 |

---

## 八、Demo 体验

`chapter_05_demo.exe`:
- 1280×720 窗口,**32×24 瓦片地图**(草地为主,有石头和水域)
- 中心一个**机甲角色**(robot3 sprite sheet,200×200 像素)
- **WASD** 控制移动:
  - 不动时播放 **idle 循环**(6 帧)
  - 移动时切换到 **walk 循环**(8 帧)
- 移动时脚下**喷出黄褐色尘土粒子**,粒子有重力+渐隐
- **空格**:在角色位置爆一个**闪光粒子**(60 粒子,放射状)
- 摄像机**平滑跟随**角色
- **标题栏**:`fps + 角色坐标 + 粒子数 + draw_calls`

---

## 九、课后练习

1. 让角色支持 **左右镜像**(走向左侧 sprite 翻转,而不是用 idle_left.png)
2. 让粒子支持 **轨迹 trail**(每个粒子记录最近 5 帧位置画一条线)
3. 实现一个 **简单导出工具**:把 Aseprite 的 `.json` 直接读取为 SpriteSheet
4. 给 Tilemap 加 **自动瓦片**(根据邻居自动选墙角 / 边)

---

## 十、常见坑

- **PNG 透明黑边**:贴图本身的边缘像素如果不是 premultiplied alpha,会显黑色光晕
- **Sprite sheet 像素裁切**:cell 不是整数倍 -> 帧边缘渗色。修复:UV 内缩半像素
- **粒子池满了**:不要 `vector.resize`,改用环形索引 + alive 标记
- **Tilemap 接缝**:线性过滤时相邻瓦片采样会跨界 -> 用 nearest 过滤,或 atlas 留 padding
- **动画跳帧**:用累加器 `m_time += dt`,不要 `now % period`

---

## 十一、延伸阅读

- [Aseprite + JSON Sprite Sheet 格式](https://www.aseprite.org/docs/sprite-sheet/)
- [Kenney 免费像素素材](https://kenney.nl/assets) 商业可用 CC0
- [GDC — Particle System Design](https://www.gdcvault.com/play/1019412/Particle-System-Design-for-Boom)
- [TheCherno — Hazel 粒子教程](https://www.youtube.com/watch?v=GK0jHlv3e3w)
- [Vampire Survivors GDC 2024](https://www.youtube.com/watch?v=H2eoB6V_5LU)

---

## 十二、下一章预告

**Ch 06 — 2D 光照 + 后效**:有了角色 + 粒子 + 世界,下一步加**动态光源**(角色举火把,墙投影)+ **Bloom**(粒子发光)+ **像素艺术保真采样**。这一章后,AIForge 的 2D 效果就到了 *Hyper Light Drifter* 级别。
