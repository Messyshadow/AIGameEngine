# AIForge Engine — AI-First Game Engine

> 全球首个**专为多模态 LLM 协作**设计的开源游戏引擎框架。
> 让任意主流大模型(Claude / GPT / DeepSeek / Codex / Qwen)高效开发游戏,Token 消耗对比 Unity 工作流降低 70%+,产出可商业发行的 2D / 3D 动作 + FPS 游戏。

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Status: Phase 1 (Ch 01-03 in progress)](https://img.shields.io/badge/Status-Phase%201-orange.svg)](#开发进度)

> **三份核心文档**
> - [README.md](README.md)(本文件)— 项目门面 / 设计思路 / 目录结构
> - [PHASES.md](PHASES.md) — 6 阶段里程碑规范
> - [CHAPTERS.md](CHAPTERS.md) — **25 章详细教学课程**

---

## 目录

- [项目目标(Vision)](#项目目标vision)
- [为什么 AIForge?](#为什么-aiforge)
- [8 个独家设计](#8-个独家设计)
- [核心设计理念](#核心设计理念)
- [技术栈](#技术栈)
- [开发阶段(6 阶段宏观规划)](#开发阶段6-阶段宏观规划)
- [25 章教学课程(微观推进)](#25-章教学课程微观推进)
- [项目目录结构](#项目目录结构)
- [快速开始(人类视角)](#快速开始人类视角)
- [快速开始(AI 视角)](#快速开始ai-视角)
- [构建说明](#构建说明)
- [开发进度](#开发进度)
- [贡献 / 协议 / 致谢](#贡献--协议--致谢)

---

## 项目目标(Vision)

| 核心目标 | 度量标准 |
|---|---|
| AI 开发效率 | 同任务 token 消耗对比 Unity 工作流 **降低 70%+** |
| 模型兼容性 | 跨 Claude / GPT / DeepSeek / Codex / Qwen **任务成功率 ≥ 80%** |
| 产出质量 | 可基于本引擎做出 **Steam 上架级** 2D 动作 + 3D FPS Demo |
| 学习友好 | 25 章渐进式教程,新手到精通 |
| 协议 | **MIT** — 商业自由使用,不收引擎税 |

---

## 为什么 AIForge?

现有游戏引擎(Unity / Unreal / Godot / Bevy)都是**为人类**设计的。AI 用它们开发游戏时面临:

| 痛点 | 原因 | AIForge 的解决方案 |
|------|------|-------------------|
| **Token 消耗巨大** | AI 需要读几千行代码才能改一个功能 | **自描述 API**:读 1 个头文件就够 |
| **频繁出错** | API 不统一,函数名记不住 | **统一命令接口**:文本命令驱动 |
| **无法增量开发** | 模块耦合,改一处影响十处 | **零耦合模块**:改一个系统不碰其他 |
| **每次重新理解项目** | 没有机器可读的项目状态 | **AIContext 自动导出**:引擎一行调用导出 markdown |
| **写代码比写配置慢** | 游戏逻辑硬编码在 C++ | **数据驱动**:JSON 配置一切 |
| **失败无法自我修正** | 错误是堆栈跟踪,AI 看不懂 | **AI 友好错误**:`unknown property 'pos'; did you mean: position?` |
| **无法 A/B 测试方案** | 没有状态快照 | **Snapshot/Replay**:AI 可 rewind 试不同方案 |
| **跨模型质量未知** | 没有兼容性数据 | **Multi-LLM Test Harness**:自带跨 5 大模型 benchmark |

---

## 8 个独家设计

| # | 创新 | 一句话说明 | 落地章节 |
|---|---|---|---|
| 1 | 统一文本命令接口 | `app.Execute("spawn zombie at 5,0,0")` 即可 | Ch 03 |
| 2 | @ai 自描述 API | 头文件标注 `@ai_summary/@ai_params/@ai_example/@ai_related/@ai_token_cost` | Ch 03 起贯穿 |
| 3 | AIContext.Export() | 一行调用导出"项目当前状态 markdown" | Ch 03 |
| 4 | AIContext.Diff() | 仅导出变化部分,大幅省 token | Ch 03 |
| 5 | Token Budget 透明化 | 每条命令标注预期 token,Context 输出可指定 budget | Ch 03 + Ch 20 |
| 6 | 错误自修正反馈 | 失败信息含"为什么 + 怎么改" | Ch 03 |
| 7 | Snapshot / Replay | AI 可 rewind 试 A/B/C 方案 | Ch 20 |
| 8 | Multi-LLM 测试矩阵 | 自带 benchmark 跨 5 大模型测命令 | Ch 20 |

---

## 核心设计理念

### 1. AI-Readable(AI 可读)
每个 API 都有 `@ai_summary` `@ai_params` `@ai_example` `@ai_related` 注释,AI 读注释就能用。

### 2. AI-Minimal(AI 最小化)
AI 完成一个任务只需要读 1-3 个文件,不需要理解整个引擎。

### 3. AI-Safe(AI 安全)
错误的 API 调用不会崩溃,而是返回可读的错误信息,AI 能自我修正。

### 4. Human-Friendly(人类友好)
不牺牲人类开发体验。人类可以正常写 C++,也可以用命令接口。

### 5. Production-Ready(可生产)
不是教学玩具。Ch 21-22 的 Demo 直接对标 Steam 上架质量。

---

## 技术栈

| 组件 | 技术 | 落地章节 |
|------|------|------|
| 窗口 / 输入 | SDL3 | Ch 01-02 |
| 2D 渲染 | OpenGL 4.5 Core(Sprite Batcher) | Ch 04-06 |
| 3D 渲染(Forward) | OpenGL 4.5 Core | Ch 07-10 |
| 现代渲染 | CSM + SSAO + TAA + Bloom + GPU 粒子 | Ch 11-13 |
| Vulkan 后端(可选) | Vulkan 1.3 + Bindless | Ch 23 |
| 物理 | PhysX 5.x | Ch 14 |
| 音频 | FMOD Core + Studio | Ch 15 |
| 骨骼动画 | Assimp + 自写 BlendTree | Ch 10 |
| UI | Dear ImGui(编辑器)+ 自写(游戏 HUD) | Ch 18-19 |
| 脚本 | JSON 配置 + Lua(可选) | Ch 03 起,Ch 24 加 Lua |
| **AI 接口** | **命令系统 + AIContext + 多模型测试套件** | **Ch 03 + Ch 20**(本引擎核心) |

---

## 开发阶段(6 阶段宏观规划)

详见 [PHASES.md](PHASES.md)。

| Phase | 名称 | 核心内容 | 对应章节 | 状态 |
|------|------|----------|------|---|
| **1** | 引擎内核 | SDL3 窗口 + 输入 + ECS + 命令系统 + JSON 加载 + AI Context 导出 | Ch 01-03 | 🔧 进行中 |
| **2** | 渲染与动画 | 2D/3D 渲染抽象 + 材质 + 骨骼动画 + 天空盒 + 粒子 | Ch 04-13 | ⏳ |
| **3** | 物理与音频 | PhysX 封装 + FMOD 封装 + 碰撞事件 + 3D 空间音效 | Ch 14-15 | ⏳ |
| **4** | 游戏系统 | 战斗系统 + AI 行为树 + 场景管理 + UI + 存档 | Ch 16-18 | ⏳ |
| **5** | 高级特性 | Vulkan 后端 + 阴影 + 后处理 + Lua 脚本 + 编辑器 + **多模型测试套件** | Ch 11-13, 19-20, 23-24 | ⏳ |
| **6** | Demo 游戏 | 2D 动作 Demo(itch.io 级)+ 3D FPS Demo(Steam Demo 级) | Ch 21-22 | ⏳ |

---

## 25 章教学课程(微观推进)

完整内容详见 [CHAPTERS.md](CHAPTERS.md)。

### 第一篇 · 引擎内核(Ch 01-03)
> SDL3 + ECS + 命令系统 + AI Context — **本引擎的"心脏"**
- **Ch 01** — Hello, Engine:窗口 + GL + ESC 退出 + FPS 标题
- **Ch 02** — ECS + Time + Input
- **Ch 03** — AI 命令协议 + Context + Token Budget(独家)

### 第二篇 · 2D 渲染管线(Ch 04-06)
- **Ch 04** — 2D Sprite Batcher + Camera2D
- **Ch 05** — 2D 动画 + 粒子 + Tilemap
- **Ch 06** — 2D 光照 + 后效

### 第三篇 · 3D Forward 管线(Ch 07-10)
- **Ch 07** — 3D Mesh + Camera3D
- **Ch 08** — PBR 材质 + 光照
- **Ch 09** — Skybox + IBL + Tonemap
- **Ch 10** — 骨骼动画 + 状态机 + 混合树

### 第四篇 · 现代渲染特性(Ch 11-13)
- **Ch 11** — Cascaded Shadow Maps + PCF
- **Ch 12** — 后处理 Stack(SSAO + Bloom + TAA)
- **Ch 13** — GPU 粒子 + Compute Shader

### 第五篇 · 物理与音频(Ch 14-15)
- **Ch 14** — PhysX 5.x(Rigid + 角色控制器 + Ragdoll)
- **Ch 15** — FMOD 3D 音频 + Adaptive Music

### 第六篇 · 游戏系统(Ch 16-18)
- **Ch 16** — 战斗系统 + 帧数据(街霸/魂系级)
- **Ch 17** — AI:行为树 + A\* + 影响图
- **Ch 18** — 场景 + UI + 存档 + 本地化

### 第七篇 · 工具 / AI 协作 — 本引擎独家(Ch 19-20)
- **Ch 19** — ImGui 编辑器(Hierarchy + Inspector + AI 命令控制台)
- **Ch 20** — **多模型测试套件 + Token Budget + Snapshot/Replay**

### 第八篇 · Demo 游戏 — 商业化(Ch 21-22)
- **Ch 21** — 2D 动作 Demo(类 Vampire Survivors,可上 itch.io)
- **Ch 22** — 3D FPS Demo(类 ULTRAKILL,可上 Steam Demo)

### 第九篇 · 高阶可选(Ch 23-25)
- **Ch 23** — Vulkan 后端 + Bindless
- **Ch 24** — Lua 脚本 + Mod 工坊
- **Ch 25** — 网络多人(P2P + 专用服)

---

## 项目目录结构

```
AIGameEngine/                            ← 项目根
│
├── README.md                             ← 项目门面(本文件)
├── PHASES.md                             ← 6 阶段里程碑
├── CHAPTERS.md                           ← 25 章教学总目录(主开发文档)
├── EXISTING_RESOURCES.md                 ← 复用 mini_game_engine 资源说明
├── PHASE1_INSTRUCTION.md                 ← Phase 1 详细执行规范
├── CONTRIBUTING.md                       ← 贡献指南
├── LICENSE                               ← MIT 协议
│
├── engine/                               ← 引擎源码(每模块解耦)
│   ├── core/                             ← 引擎核心(Ch 01-02)
│   │   ├── App.h/cpp                     ← 应用生命周期 Init/Run/Shutdown
│   │   ├── Window.h/cpp                  ← SDL3 窗口 + OpenGL Context
│   │   ├── Input.h/cpp                   ← 键盘 / 鼠标 / 手柄统一接口
│   │   ├── Time.h/cpp                    ← deltaTime / FPS / 固定步长
│   │   ├── ECS.h/cpp                     ← Entity / Component / World
│   │   ├── ResourceManager.h/cpp         ← 路径解析 + 加载 + 引用计数
│   │   └── Math.h/cpp                    ← Vec2/3/4 + Mat3/4(Ch 06+ 扩展)
│   │
│   ├── command/                          ← AI 命令系统 ★ 核心创新(Ch 03)
│   │   ├── CommandParser.h/cpp           ← 文本命令解析("spawn x at y,z,w")
│   │   ├── CommandRegistry.h/cpp         ← 命令注册 / 派发
│   │   └── AIContext.h/cpp               ← AI 上下文导出器(Export + Diff)
│   │
│   ├── render/                           ← 渲染抽象层(Ch 04+)
│   │   ├── RenderAPI.h                   ← 渲染接口(抽象基类)
│   │   ├── backend_gl/                   ← OpenGL 实现
│   │   ├── backend_vk/                   ← Vulkan 实现(Ch 23)
│   │   ├── Shader.h/cpp                  ← Shader 管理
│   │   ├── Mesh.h/cpp                    ← 网格数据 + 工厂
│   │   ├── Material.h                    ← PBR 材质(Ch 08)
│   │   ├── Texture.h/cpp                 ← 纹理加载(stb_image)
│   │   ├── Camera.h                      ← FPS / Orbit / TPS 三模式
│   │   ├── Skybox.h                      ← 天空盒 + IBL(Ch 09)
│   │   ├── ParticleSystem.h              ← CPU/GPU 粒子(Ch 13)
│   │   └── PostProcess.h                 ← Bloom/SSAO/TAA(Ch 12)
│   │
│   ├── animation/                        ← 动画系统(Ch 10)
│   │   ├── Animator.h                    ← 骨骼蒙皮
│   │   ├── AnimStateMachine.h            ← 状态机
│   │   └── BlendTree.h                   ← 混合树
│   │
│   ├── physics/                          ← 物理抽象(Ch 14)
│   │   ├── PhysicsAPI.h                  ← 抽象接口
│   │   └── backend_physx/                ← PhysX 实现
│   │
│   ├── audio/                            ← 音频抽象(Ch 15)
│   │   ├── AudioAPI.h
│   │   └── backend_fmod/                 ← FMOD 实现
│   │
│   ├── ai/                               ← 游戏 AI(Ch 17)
│   │   ├── BehaviorTree.h                ← 行为树
│   │   ├── Pathfinding.h                 ← A\* + NavMesh
│   │   └── InfluenceMap.h                ← 影响图
│   │
│   ├── gameplay/                         ← 游戏玩法(Ch 16)
│   │   ├── CombatSystem.h                ← 战斗 / 帧数据 / Cancel
│   │   ├── HealthComponent.h
│   │   └── WeaponComponent.h
│   │
│   ├── ui/                               ← UI 系统(Ch 18-19)
│   │   ├── GameUI.h                      ← HUD / 飘字 / 血条
│   │   └── DebugUI.h                     ← ImGui 调试面板
│   │
│   └── scene/                            ← 场景管理(Ch 18)
│       ├── SceneManager.h                ← 场景切换
│       ├── SceneLoader.h/cpp             ← JSON 场景加载
│       └── TriggerSystem.h               ← 触发器
│
├── examples/                             ← 25 章教学交付物 ★(代码 + 教程同处)
│   ├── 01_hello_window/
│   │   ├── main.cpp                      ← 本章可跑的示例
│   │   ├── README.md                     ← 本章教程(图文 + 代码片段 + 设计思路)
│   │   └── (后续可加 HOMEWORK.md / assets/)
│   ├── 02_ecs_demo/
│   ├── 03_command_demo/
│   ├── 04_2d_sprite_batcher/
│   ├── 05_2d_animation_particles_tilemap/
│   ├── 06_2d_lighting_postfx/
│   ├── 07_3d_mesh_camera/
│   ├── 08_pbr_materials_lighting/
│   ├── 09_skybox_ibl_tonemap/
│   ├── ...                               ← 10-25 章按需创建
│   └── 22_fps_arena_demo/
│
├── tools/                                ← 开发工具
│   ├── ai_context_export/
│   │   └── export.cpp                    ← 命令行工具:导出 AI 上下文
│   ├── llm_bench/                        ← Multi-LLM 测试套件 ★(Ch 20)
│   │   ├── tasks/                        ← 标准任务集
│   │   ├── runners/                      ← Claude/GPT/DeepSeek/Codex/Qwen 客户端
│   │   └── reports/                      ← 各模型 success rate / token 消耗
│   └── asset_pipeline/                   ← 资源处理管线
│       ├── atlas_packer/                 ← Sprite Atlas 烘焙
│       └── cubemap_baker/                ← Cubemap 烘焙
│
├── templates/                            ← AI 任务模板
│   ├── new_entity.json                   ← 创建新实体
│   ├── new_enemy.json                    ← 创建新敌人(Ch 17)
│   ├── new_weapon.json                   ← 创建新武器(Ch 16)
│   ├── new_level.json                    ← 创建新关卡(Ch 18)
│   ├── new_skill.json                    ← 创建新技能(Ch 16)
│   └── new_ui_element.json               ← 创建新 UI 元素(Ch 18)
│
├── data/                                 ← 游戏资源
│   ├── config/                           ← JSON 配置
│   │   └── engine.json                   ← 引擎默认配置
│   ├── shaders/                          ← Shader 源文件
│   │   ├── standard.vs/fs                ← 标准 PBR
│   │   ├── skinning.vs/fs                ← 蒙皮
│   │   ├── skybox.vs/fs
│   │   └── particle.vs/fs
│   ├── models/                           ← 3D 模型(FBX/OBJ)
│   ├── textures/                         ← 纹理
│   ├── audio/                            ← 音频
│   └── levels/                           ← 关卡 JSON
│
├── game/                                 ← 游戏逻辑(AI 主要工作区)
│   ├── entities/                         ← 实体定义(JSON)
│   ├── levels/                           ← 关卡配置(JSON)
│   └── systems/                          ← 自定义游戏系统
│
│
├── docs/                                 ← 设计文档
│   ├── AI_GUIDE.md                       ← AI 开发者指南(怎么用命令接口)
│   ├── ARCHITECTURE.md                   ← 架构说明
│   ├── API_REFERENCE.md                  ← API 参考(Ch 04+ 增长)
│   └── images/                           ← 文档图片
│
├── depends/                              ← 第三方库(部分从 mini_game_engine 复制)
│   ├── glad/                             ← OpenGL 加载器(已复制)
│   ├── stb/                              ← stb_image.h(已复制)
│   ├── imgui/                            ← Dear ImGui(Ch 19 复制)
│   ├── assimp/                           ← Assimp(Ch 10 复制)
│   ├── fmod/                             ← FMOD(Ch 15 复制)
│   └── physx/                            ← PhysX(Ch 14 复制)
│
├── build/                                ← CMake 构建产物(.gitignore)
│   └── bin/Release/
│       ├── chapter_01_demo.exe           ← Ch 01 可跑 demo ✅ 已就绪
│       ├── SDL3.dll                      ← 自动复制
│       ├── chapter_02_demo.exe           ← 后续章节生成
│       └── ...
│
└── CMakeLists.txt                        ← 顶层构建(SDL3 FetchContent + 所有 target)
```

> **设计哲学**:每个目录职责清晰,AI 改一个系统不需要碰其他系统。`engine/` 下每个子目录都有自己的命名空间(`AIForge::core`, `AIForge::render` 等),通过抽象接口跨目录通信。

---

## 快速开始(人类视角)

```cpp
#include "AIForge.h"

int main() {
    auto app = AIForge::App::Create("data/config/engine.json");
    if (!app || !app->Init()) return 1;

    auto* player = app->GetWorld()->Spawn("player");
    player->GetComponent<AIForge::Transform>()->position = {0, 0, 0};
    // 后续章节会加 RigidBody / Animator / Health 等组件

    app->Run();   // 阻塞直到关闭
    return 0;
}
```

## 快速开始(AI 视角)

AI 不写 C++,直接发命令:

```
spawn player at 0,0,0
add rigidbody to player mass 70           ← Ch 14 起支持
play animation idle on player loop true   ← Ch 10 起支持
load level levels/stage1.json             ← Ch 18 起支持
run
```

或更紧凑的批量:

```python
app.execute_batch([
    "spawn player at 0,0,0",
    "spawn zombie at 5,0,0",
    "spawn zombie at 5,0,3",
])
ctx = app.export_context(diff=True, max_tokens=2000)
# 把 ctx 喂给下一轮对话
```

---

## 构建说明

### 依赖
- **CMake** ≥ 3.20
- **C++17 编译器**:MSVC 2022(Windows 主推) / Clang 14+ / GCC 11+
- **Git** — CMake FetchContent 需要(自动下载 SDL3 / nlohmann_json)
- **网络**:首次配置约下载 100 MB(SDL3 源码 + json 单头),之后离线编译

### 第三方库(全部自动获取,不用手装)
| 库 | 来源 | 用途 |
|---|---|---|
| SDL3 (release-3.2.0) | CMake FetchContent | 窗口/输入/手柄 |
| nlohmann/json (v3.11.3) | CMake FetchContent | JSON 解析 |
| glad (vendored) | `depends/glad/` | OpenGL 函数加载器 |
| stb_image | `depends/stb/` | 图片解码(后续章节用) |

### 构建步骤(Windows / PowerShell)
```powershell
# 1) 配置(首次约 4 分钟,会下载 SDL3)
cmake -S . -B build -G "Visual Studio 17 2022" -A x64

# 2) 编译(首次约 5-8 分钟,SDL3 第一次编;后续增量编译只要几秒)
cmake --build build --config Release

# 3) 运行某一章 demo —— .exe 都在 build/bin/Release/ 下
.\build\bin\Release\chapter_01_demo.exe
.\build\bin\Release\chapter_02_demo.exe
```

### 构建步骤(Linux / macOS)
```bash
cmake -S . -B build
cmake --build build --config Release -j
./build/bin/chapter_01_demo
```

### 单章增量编译(开发推荐)
只重编当前章节,几秒搞定:
```powershell
cmake --build build --config Release --target chapter_02_demo
```

### 可执行文件位置(重要)
**所有 `chapter_XX_demo.exe` 都在同一个目录**,配套的 `SDL3.dll` 自动复制过去:
```
e:\git_project\games\engine\AIGameEngine\build\bin\Release\
├── chapter_01_demo.exe
├── chapter_02_demo.exe
├── chapter_03_demo.exe   ← 后续章节继续往这里加
├── ...
└── SDL3.dll
```

> ⚠️ 控制台型 demo(如 Ch 03 REPL)请从 PowerShell 启动,**不要双击**,否则看不到控制台输出。
> Ch 14 起需要本地有 PhysX,Ch 15 起需要 FMOD,届时本说明会更新。

---

## 开发进度

| 章节 | 教程文档 | 代码 | 编译验证 | 用户验收 | 状态 |
|---|---|---|---|---|---|
| **Ch 01 — Hello, Engine** | ✅ | ✅ | ✅ | ✅ | **✅ 完整交付** |
| **Ch 02 — ECS + Time + Input** | ✅ | ✅ | ✅ | ✅ | **✅ 完整交付** |
| Ch 03 — AI 命令协议 | ✅ 占位 | ⏳ 引擎 command/ 已就绪 | — | — | ⏳ 等启动 |
| Ch 04 — 2D Sprite Batcher | ✅ 占位 | ❌ | — | — | ⏳ |
| Ch 05 — 2D 动画 / 粒子 / Tilemap | ✅ 占位 | ❌ | — | — | ⏳ |
| Ch 06 — 2D 光照 + 后效 | ✅ 占位 | ❌ | — | — | ⏳ |
| Ch 07 — 3D Mesh + Camera | ✅ 占位 | ❌ | — | — | ⏳ |
| Ch 08 — PBR 材质 / 光照 | ✅ 占位 | ❌ | — | — | ⏳ |
| Ch 09 — Skybox + IBL + Tonemap | ✅ 占位 | ❌ | — | — | ⏳ |
| Ch 10-25 | — | ❌ | — | — | 后续做到再写 |

> 当前里程碑:**第一篇(引擎内核)进行中**,Ch 01-02 已通过,Ch 03 待开始。
> 详细 milestone 见 [PHASES.md](PHASES.md);章节级教学计划见 [CHAPTERS.md](CHAPTERS.md)。

---

## 贡献 / 协议 / 致谢

### 贡献
欢迎所有"AI 开发工具"和"现代游戏引擎"方向的开发者参与。详见 [CONTRIBUTING.md](CONTRIBUTING.md)。

### 协议
**MIT License** — 自由使用,商业友好,不收引擎税。

### 致谢
- **SDL3** — 窗口与输入
- **OpenGL / Vulkan** — 图形渲染
- **PhysX** — 物理引擎
- **FMOD** — 音频引擎
- **Assimp** — 模型加载
- **Dear ImGui** — 调试 UI 与编辑器
- **stb** — 图像加载
- **glad** — OpenGL 加载器
- **nlohmann/json** — JSON 库
- **Mini Game Engine 教程项目** — 技术验证与素材来源

---

**项目愿景**:让 AI 开发游戏像人类聊天一样自然,让独立游戏团队能用 1 个程序员 + 几个 LLM 做出 Steam 上架级别的游戏。
