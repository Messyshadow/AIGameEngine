# Chapter 05 — 2D 动画 + 粒子 + Tilemap

> **本章定位**:让 2D 画面"动起来" — sprite 动画 / 粒子拖尾 / 瓦片地图。

---

## 本章目标
1. **Sprite Sheet 动画**:从单张图按帧切片播放
2. **Sprite Atlas 烘焙**:多张小图打包成 atlas + 自动生成 UV
3. **粒子系统(CPU 版)**:支持发射器 / 重力 / 颜色渐变 / 缩放渐变
4. **Tilemap**:数据驱动的瓦片地图,带相机裁剪
5. Demo:顶视角小角色在瓦片地图上跑动 + 鞋底粒子拖尾

## 跑出来的 Demo
`build/Release/chapter_05_demo.exe` — 顶视角 64×64 瓦片地图,WASD 控制角色 8 帧 walk 动画播放,角色脚下持续生成尘土粒子(随机方向飘散后渐隐)。

## 学到的前沿技术
- **Frame Animation**:精灵图分帧 + 时间累加器 + 循环模式
- **Texture Atlas Packing**:bin packing 算法(stb_rect_pack 或自己写贪心)
- **Particle Pool**:为什么不能 new/delete 每个粒子(分配开销)
- **SoA vs AoS**:粒子数据结构布局对性能的影响
- **Tilemap Chunking**:大地图按 chunk 加载 + 视锥裁剪

## 背景知识

**Sprite 动画**是 2D 游戏的灵魂。任天堂在 Game Boy 时代就用这个套路。现代实现:`Aseprite` / `TexturePacker` 导出的 .json + .png 是行业标准。

**粒子系统**让画面"活"起来 — 拖尾、爆炸、烟雾、光环。Unity ParticleSystem / Unreal Niagara 都是这个原理(GPU 版本性能更高,留到 Ch 13)。

**Tilemap** 是大地图的解法 — 不要给每个瓦片做一个 sprite,直接当成"一张可索引的大图"。

## 架构设计

```
SpriteAnimation
 ├─ frames: vector<UV>     (从 atlas 中切出的帧)
 ├─ fps: float
 ├─ loop: bool
 └─ Tick(dt) → 当前 UV

ParticleEmitter
 ├─ shape: Point/Circle/Box
 ├─ rate: float (粒子/秒)
 ├─ lifetime: range
 ├─ velocity: range
 ├─ colorOverLifetime: gradient
 ├─ sizeOverLifetime: curve
 └─ pool: vector<Particle>(预分配)

Tilemap
 ├─ tileSize: 16/32 px
 ├─ width × height
 ├─ atlas: Texture(瓦片图)
 ├─ tiles: vector<uint16_t>  (瓦片索引数组)
 └─ Render(camera) → 用 SpriteBatcher 提交可见瓦片
```

## 涉及源文件
- `engine/render/SpriteAnimation.h/cpp`
- `engine/render/ParticleSystem.h/cpp`(本章 CPU 版)
- `engine/render/Tilemap.h/cpp`
- `tools/asset_pipeline/atlas_packer/` — 离线 atlas 烘焙工具
- `data/shaders/particle.vs/fs`

## 课后练习
1. 加 `Aseprite` JSON 导入支持
2. 让粒子支持拖尾(每个粒子记录最近 N 帧的位置)
3. Tilemap 加自动瓦片(根据邻居自动选择墙角/边)

## 常见坑
- Frame Animation 用 `delta_accumulator >= 1/fps` 而不是 `now % period`,后者帧率不稳时会跳帧
- 粒子池预分配上限要够,否则瞬间溢出会卡顿
- Tilemap 渲染时只画屏幕上的瓦片,不要全画

## 延伸阅读
- [Aseprite + JSON Sprite Sheet](https://www.aseprite.org/docs/sprite-sheet/)
- [stb_rect_pack — atlas packing](https://github.com/nothings/stb/blob/master/stb_rect_pack.h)
- [Unity ParticleSystem 文档](https://docs.unity3d.com/Manual/ParticleSystemMain.html) — 模块化设计参考
- [Cellular Automata for Cave Generation](https://www.roguebasin.com/index.php/Cellular_Automata_Method_for_Generating_Random_Cave-Like_Levels)

## 下一章预告
**Ch 06 — 2D 光照 + 后效**:用 normal map 给 2D 加伪 3D 光照,实现像 *Hyper Light Drifter* 那种像素艺术质感。
