# AIForge Engine — 教学课程总目录(CHAPTERS.md)

> **本文件是 AIForge 项目的"教学骨架"**:25 章的开发课程,每章交付一个可跑的 demo + 一份图文教程 + 高注释代码。配合 [PHASES.md](PHASES.md)(6 阶段里程碑)和 [README.md](README.md)(项目概览)使用。

---

## 项目目标(Vision)

AIForge 是**第一个为多模态 LLM 协作量身设计的开源游戏引擎**。

| 核心目标 | 度量标准 |
|---|---|
| AI 开发效率 | 同任务 token 消耗对比 Unity 工作流 **降低 70%+** |
| 模型兼容性 | 跨 Claude / GPT / DeepSeek / Codex / Qwen **任务成功率 ≥ 80%** |
| 产出质量 | 可基于本引擎做出 **Steam 上架级** 2D 动作 + 3D FPS Demo |
| 学习友好 | 25 章渐进式教程,新手到精通 |
| 协议 | **MIT** — 商业自由使用 |

---

## 为什么 AIForge 与现有引擎不同?

### 现有引擎的痛点(以 AI 开发者视角)

| 引擎 | 问题 |
|---|---|
| Unity / Unreal | 庞大且为人类设计;AI 要读几千行代码 / 几百份文档才能改一个功能 |
| Godot | 比 Unity 轻,但 GDScript 不是大模型主训语料,出错率高 |
| Bevy / Macroquad | 更新快但接口不稳定;AI 训练数据滞后 |
| 自研引擎(无 AI 接口)| 没有 AI 协作设计,token 消耗失控 |

### AIForge 的 8 个独家设计

| 创新 | 一句话说明 | 落地章节 |
|---|---|---|
| **统一文本命令接口** | `app.Execute("spawn zombie at 5,0,0")` 即可,所有引擎能力都有命令对应 | Ch 03 |
| **@ai 自描述 API** | 头文件标注 `@ai_summary/@ai_params/@ai_example/@ai_related/@ai_token_cost` | Ch 03 起贯穿全程 |
| **AIContext.Export()** | 一行调用导出"项目当前状态 markdown",AI 不读源码就懂 | Ch 03 |
| **AIContext.Diff()** | 仅导出"上次导出后变化的部分",大幅省 token | Ch 03 |
| **Token Budget 透明化** | 每条命令标注预期 token 成本,Context 输出可指定 budget | Ch 03 + Ch 20 |
| **错误自修正反馈** | 失败返回 `"unknown property 'pos'; did you mean: position?"` | Ch 03 |
| **Snapshot / Replay** | AI 可 rewind 状态,试 A/B/C 不同方案 | Ch 20 |
| **Multi-LLM 兼容性测试** | 自带 benchmark 跨 5 大模型测命令成功率 + token 消耗 | Ch 20 |

---

## 完整 25 章课程

### 第一篇 · 引擎内核(Ch 01-03)

| Ch | 标题 | 跑出来的 Demo | 学到的前沿技术 |
|----|---|---|---|
| **01** | Hello, Engine | SDL3 蓝色窗口 + ESC 退出 + 标题显示 FPS | SDL3 事件循环 / OpenGL 4.5 Core / glad2 加载器 / VSync / 双缓冲 |
| **02** | ECS + Time + Input | 控制台打印 10 个旋转中实体的位置 | 数据导向 ECS / 智能指针所有权 / 帧无关固定步长 / 输入边缘 vs 电平触发 |
| **03** | AI 命令协议 + Context 系统 | REPL:`spawn zombie at 5,0,0` / `export context` / **`token budget`** | 文本协议设计 / `std::function` 派发 / **Token Budget 雏形** / `@ai_*` 注释规范 |

### 第二篇 · 2D 渲染管线(Ch 04-06)

| Ch | 标题 | 跑出来的 Demo | 学到的前沿技术 |
|----|---|---|---|
| **04** | 2D Sprite Batcher + Camera2D | 屏幕上 1 万个移动 sprite,稳定 60fps | VBO/VAO/EBO / 自动批渲染 / 正交投影 / Indirect Draw 入门 |
| **05** | 2D 动画 + 粒子 + Tilemap | 顶视角 2D 角色 + 粒子拖尾 + 瓦片地图 | Sprite Atlas 烘焙 / 粒子池 / 数据驱动 Tilemap / Frame Animation |
| **06** | 2D 光照 + 后效 | Normal-mapped 2D + 2D 阴影投射 + 像素艺术保真 | 2D 法线贴图 / 2D Shadow Casting / 像素完美采样 |

### 第三篇 · 3D Forward 管线(Ch 07-10)

| Ch | 标题 | 跑出来的 Demo | 学到的前沿技术 |
|----|---|---|---|
| **07** | 3D Mesh + Camera3D | 旋转的彩色立方体 + 自由飞行摄像机 | MVP 矩阵 / 透视投影 / Frustum Culling 入门 / FPS / Orbit / TPS Camera |
| **08** | PBR 材质 + 光照 | 5 个 PBR 材质球(金属/木头/砖) + 方向光 | **Cook-Torrance BRDF** / 金属-粗糙度工作流 / Normal/AO/Roughness 贴图 / 能量守恒 |
| **09** | Skybox + IBL + Tonemap | HDR 天空盒 + 镜面金属反射环境 + ACES 色调映射 | Cubemap / **Image-Based Lighting**(行业标准)/ Gamma 校正 / ACES |
| **10** | 骨骼动画 + 状态机 + 混合树 | X-Bot 角色 idle / run / attack 切换,带过渡 | Assimp 加载 / 双四元数蒙皮 / **Animation Blend Tree**(Unity Mecanim 同款) |

### 第四篇 · 现代渲染特性(Ch 11-13)

| Ch | 标题 | 跑出来的 Demo | 学到的前沿技术 |
|----|---|---|---|
| **11** | Cascaded Shadow Maps + PCF | 大场景下角色和建筑都有自然阴影,无 acne | **CSM 4 级级联** / PCF 软阴影 / Bias 调优 / Peter-Panning 修复 |
| **12** | 后处理 Stack | 同场景对比 Bloom / SSAO / TAA 开关 | **SSAO**(屏幕空间环境光遮蔽)/ Bloom / **TAA**(时序抗锯齿)/ Vignette |
| **13** | GPU 粒子 + Compute Shader | 屏幕上 100 万粒子,实时风场互动 | **Compute Shader** / SSBO / GPU 粒子模拟 / 间接绘制 |

### 第五篇 · 物理与音频(Ch 14-15)

| Ch | 标题 | 跑出来的 Demo | 学到的前沿技术 |
|----|---|---|---|
| **14** | PhysX 5.x 物理 | 100 个箱子从天上掉落 + 第一/三人称角色控制器 + 布娃娃倒下 | Rigid Body / **Character Controller** / Raycast / Trigger / **Ragdoll 入门** |
| **15** | FMOD 3D 音频 + 音乐分层 | BGM 战斗/探索切换 + 3D 空间音效 | 3D Attenuation / Bus 路由 / **Adaptive Music** / Doppler |

### 第六篇 · 游戏系统(Ch 16-18)

| Ch | 标题 | 跑出来的 Demo | 学到的前沿技术 |
|----|---|---|---|
| **16** | 战斗系统 + 帧数据 | 玩家轻/重攻击,有受击硬直、连段、cancel | HitStop / HitFreeze / **帧数据 JSON** / Cancel 系统(街霸/魂系同款) |
| **17** | AI:行为树 + A\* + 影响图 | 3 个会追击/攻击/逃跑的僵尸 | 黑板模式 / **Behavior Tree** / A\* 寻路 / NavMesh 入门 / **Influence Map** |
| **18** | 场景 + UI + 存档 + 本地化 | 切场景 + HUD + 对话框 + F5/F9 存读档 + 中英切换 | 异步场景加载 / IM-style UI / JSON 存档 / **i18n** |

### 第七篇 · 工具与 AI 协作工程 — 本引擎独家(Ch 19-20)

| Ch | 标题 | 跑出来的 Demo | 学到的前沿技术 |
|----|---|---|---|
| **19** | ImGui 编辑器 | 可拖动场景层级 + Inspector + 命令控制台,**实时改场景** | Dear ImGui Docking / 简版反射 / Asset Browser / **AI 命令控制台** |
| **20** | **多模型测试套件 + Token Budget 工具** | 跑同一任务,输出 5 大模型成功率 + token 消耗对比表 | **Multi-LLM Test Harness** / Token Budget API / **Snapshot & Replay** / 回归测试 |

### 第八篇 · Demo 游戏(Ch 21-22)— 商业化目标

| Ch | 标题 | 跑出来的 Demo | 商业目标 |
|----|---|---|---|
| **21** | 2D 动作 Demo:类 Vampire Survivors | 完整 wave 系统 + 升级树 + boss 战 | **可上 itch.io** |
| **22** | 3D FPS Demo:类 ULTRAKILL 小型 arena | 武器系统 / 敌人 AI / 完整关卡 | **可上 Steam Demo** |

### 第九篇 · 高阶可选(Ch 23-25,按需启用)

| Ch | 标题 | 何时做 |
|----|---|---|
| 23 | **Vulkan 后端** + Bindless 资源 + 命令队列 | 需要更高图形性能时 |
| 24 | Lua 脚本 + Mod 支持 + Workshop 集成 | 想做 UGC / 创意工坊时 |
| 25 | 网络多人(P2P + 专用服 + 状态同步) | 想做联机游戏时 |

---

## 每章交付物标准

每一章都按 mini_game_engine 的标准交付:

```
examples/XX_chapter_name/
├── README.md          ← 教程文档(图+文+代码片段),带"为什么这样设计"的解释
├── HOMEWORK.md        ← 课后练习(让你动手改 3-5 个小地方)
├── main.cpp           ← 本章可跑的示例程序(也可拆 src/ + main.cpp)
└── assets/            ← 本章用到的素材(模型/贴图/音频)
```

build 后:`build/bin/Release/chapter_XX_demo.exe` 双击就能跑。

教程文档(README.md)结构:
1. **本章目标**:跑出来什么、学会什么
2. **背景知识**:为什么这个特性重要(对比业界做法)
3. **架构设计**:用 ASCII 图或 Mermaid 画出来
4. **逐步实现**:分 3-5 个小节,每节带可对照的代码片段
5. **常见坑位**:作者和 AI 都会踩的坑,提前列出
6. **延伸阅读**:行业经典论文 / GPU Gems 章节 / 引擎开源代码链接
7. **下一章预告**

---

## 现代游戏引擎技术清单(贯穿 25 章)

### 架构与系统
- ECS / 数据导向设计(DOD)
- 抽象渲染后端(OpenGL/Vulkan 双跑)
- 资源管线(Asset Pipeline)
- 事件总线 / 命令系统 / 信号槽
- 热重载(Hot Reload)
- 序列化与反序列化(JSON / Binary)

### 渲染技术
- OpenGL 4.5 Core(主)/ Vulkan 1.3(可选)
- **PBR**(基于物理的渲染)
- **IBL**(基于图像的光照)
- **CSM**(级联阴影贴图)
- **SSAO**(屏幕空间环境光遮蔽)
- **TAA**(时序抗锯齿)
- **Bloom / Tonemap / Color Grading**
- **GPU 粒子 + Compute Shader**
- 间接绘制(Indirect Draw)/ Bindless 资源

### 动画
- 骨骼蒙皮(双四元数)
- 状态机 / **混合树**(Blend Tree)
- IK(逆向运动学,后期)
- Root Motion

### 物理
- PhysX 5.x 集成
- 角色控制器(Character Controller)
- Raycast / Sweep / Overlap
- Trigger 区域
- **Ragdoll**(布娃娃)

### 音频
- FMOD Core + Studio
- 3D 衰减 / Doppler
- Bus 路由
- **Adaptive Music**(自适应音乐)

### 游戏 AI
- 行为树(Behavior Tree)
- 黑板(Blackboard)
- A\* 寻路 + NavMesh
- **影响图**(Influence Map)
- 状态机 vs 行为树 vs 效用 AI

### 工具链
- CMake FetchContent / vcpkg(可选)
- ImGui Editor(Docking + 反射)
- 资源烘焙(Sprite Atlas / Cubemap / Mesh Optimizer)
- 多模型测试套件(本引擎独家)

---

## 三个 AAA 级独家设计详解

### 1. Multi-LLM 兼容性测试矩阵(Ch 20)

```
tools/llm_bench/
├── tasks/
│   ├── spawn_5_zombies.task        ← 标准任务定义
│   ├── load_level_and_setup.task
│   └── implement_dash_skill.task
├── runners/
│   ├── claude_runner.py            ← Claude API 客户端
│   ├── gpt_runner.py               ← OpenAI API
│   ├── deepseek_runner.py          ← DeepSeek API
│   ├── codex_runner.py             ← Codex API
│   └── qwen_runner.py              ← 通义千问
├── reports/                         ← 各模型 success rate + token 消耗对比
└── README.md                        ← 怎么自己加新模型
```

**价值**:开发者选 LLM 时有数据依据。"用 DeepSeek 是不是会比 Claude 差?差多少?"——本套件给答案。

### 2. Token Budget 透明化(Ch 03 + Ch 20)

每个命令在 `@ai_token_cost` 标注预期 token 成本,AIContext 输出可指定 budget:

```cpp
auto ctx = AIContext::Export(*app, {
    .maxTokens = 2000,                  // 总预算
    .diffSinceLastExport = true,        // 只输出变化部分
    .includeResources = false,          // 不导出资源列表(已知就跳过)
});
// ctx.tokenCount = 实际消耗的 token 数
```

AI 永远知道自己烧了多少,主动批量化操作。

### 3. Snapshot / Replay 用于 AI 试错(Ch 20)

```cpp
auto snap = engine.Snapshot("before_combat_design");

// AI 试方案 A
engine.Execute("spawn boss at 0,0,0");
engine.Execute("set boss.hp 1000");
// ... 测试

engine.Restore(snap);
// AI 试方案 B(完全干净的状态)

engine.Execute("spawn boss_v2 at 0,0,0");
// ...

engine.Restore(snap);
// AI 试方案 C
// ...
```

**Unity / Unreal 都没有这个能力**。这让 AI 可以"探索-比较-选优",而不是一次性写到底。

---

## 当前进度 & 下一步

| Ch | 状态 |
|----|---|
| Ch 01 引擎核心代码 | ✅ 完成(Math/ECS/Time/Window/Input/App/Command/AIContext/ResourceManager,约 1500 行) |
| Ch 01 demo + 教程 + build | ✅ 完成(`examples/01_hello_window/` + `build/bin/Release/chapter_01_demo.exe`) |
| Ch 02-09 教程占位文档 | ✅ 写入 `examples/02-09_*/README.md`(代码逐章交付) |
| Ch 02 实施 | ⏳ 等用户验证 Ch 01 后启动 |
| Ch 04 起 | 未开始 |

**当前节奏**:你审完本文档 → 我完成 Ch 01-03 的全套交付(代码 + 教程 + demo + build + push) → 你跑通 → 进入 Ch 04。

---

## 风险提示(透明)

| 风险 | 缓解 |
|---|---|
| 完整 25 章周期长(数月) | 每章独立可跑,**任何时刻停下都有可商业化的成果** |
| Vulkan 后端复杂度高 | 做成可选(Ch 23),OpenGL 路径必须始终可用 |
| 跨模型兼容性可能有差异 | Ch 20 测试套件提前发现,通过命令系统降低对话差异 |
| FMOD 商业授权 | <$200K 营收免费,引擎开源友好;商业大作可换 OpenAL/SoLoud |
| 网络多人复杂 | Ch 25 标记可选,不影响主线 |

---

## 术语小词典(给非引擎背景读者)

| 术语 | 简释 |
|---|---|
| **ECS** | Entity-Component-System,把"是什么"和"做什么"分开,游戏行业主流架构 |
| **PBR** | Physically Based Rendering,让金属/木头/塑料一眼能分辨的光照模型 |
| **IBL** | Image-Based Lighting,用环境贴图当光源,真实感的关键 |
| **TAA** | Temporal Anti-Aliasing,用上一帧抹锯齿,几乎所有 3A 都用 |
| **CSM** | Cascaded Shadow Maps,大场景阴影标准做法 |
| **NavMesh** | 导航网格,A\* 寻路的预计算地图 |
| **HitStop** | 攻击命中瞬间画面冻结几帧,提升打击感 |
| **Bindless** | 着色器直接索引大量贴图/缓冲,Vulkan/DX12 特性 |

---

**最后一句**:这个引擎是为"AI + 人类协作"设计的。每一章我都会写得让你看完能讲给朋友听,代码看完能上手改。我们一起做一件市面上还没人做过的事。

— Claude Opus 4.7,2026
