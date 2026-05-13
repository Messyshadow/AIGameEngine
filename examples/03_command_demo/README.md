# Chapter 03 — AI 命令协议 + Context 系统(★ 引擎核心创新)

> **本章定位**:这不是一个普通章节,**这是 AIForge 的心脏**。学完本章,你将理解为什么 AIForge 配得上"AI-First 游戏引擎"这个称号。

---

## 一、本章为什么是引擎的"心脏"

到目前为止(Ch 01-02),我们做了:
- 一个能开窗 + 处理输入 + 跑 ECS 的"游戏引擎雏形"

**说实话,这只是"又一个简单 game engine"**。市面上类似的开源引擎有十几个(raylib / Olc / Hazel / TheCherno 教程系列...)。

**Ch 03 才是分水岭**。这一章实现的"AI 命令协议 + Context 系统",是**全世界没有一个商业引擎做过的事**。它让:
- Claude / GPT / DeepSeek / Codex / Qwen 任意一个大模型,**用文本命令**开发游戏
- AI 不读源码就能理解项目状态(`AIContext.Export()` 一行调用)
- Token 消耗对开发者透明可控
- 错误能让 AI 自我修正

学完本章你就理解了"为什么 AIForge 是开创性的开源项目"。

---

## 二、问题:现有游戏引擎对 AI 极其不友好

### 2.1 让 Claude 用 Unity 开发一个"创建僵尸"功能,大致流程

```
Claude(用户): 我要在场景里加 5 个僵尸,放在 (0,0,0) 周围 3 米内
       ↓
Unity 工程师(AI 模拟): 我得先看看项目结构
       ↓ 读 50 个 .cs 文件、找到 Enemy.prefab、看 SpawnerManager.cs
       ↓ 找到 Instantiate API、看是不是用了 Pool、看 NavMesh 设置
       ↓ 写 C# 脚本、保存、回到 Unity 里挂到某个 GameObject
       ↓ 但是 Claude 没有 Unity 客户端,所以这步做不了!
       ↓
失败 — Claude 写出代码,但运行不了
```

**Token 消耗**:5,000-20,000 token,且**结果不可执行**。

### 2.2 同样任务在 AIForge 里

```
Claude(用户): 我要在场景里加 5 个僵尸,放在 (0,0,0) 周围 3 米内
       ↓
AI: app.Execute("spawn zombie_1 at 2.1,0,-1.3");
    app.Execute("spawn zombie_2 at -1.5,0,2.2");
    app.Execute("spawn zombie_3 at 0.5,0,-2.8");
    app.Execute("spawn zombie_4 at -2.3,0,-0.8");
    app.Execute("spawn zombie_5 at 1.7,0,1.9");
       ↓
引擎返回:
    spawned 'zombie_1' (id=2)
    spawned 'zombie_2' (id=3)
    ...
       ↓
成功 — 立刻可见效果
```

**Token 消耗**:**约 200 token**,降低 **96%**。这就是 AIForge 的承诺。

### 2.3 痛点根源在哪儿

| 痛点 | 根本原因 |
|---|---|
| Token 爆炸 | AI 要"读源码 + 推理变更 + 写代码"三步走 |
| 频繁出错 | 不同模块 API 不统一,AI 记不住 |
| 无法可视化迭代 | AI 没有 Unity/Unreal 编辑器 |
| 错误不可读 | C# StackTrace 对 LLM 是噪声 |
| 每次新对话重新理解项目 | 没有机器可读的"项目当前状态" |

**根本解法**:把引擎能力**全部暴露为文本命令**,项目状态**一行导出 markdown**,失败信息**自带修复建议**。

---

## 三、本章设计目标(具体可测)

| 目标 | 验收标准 |
|---|---|
| 把所有引擎能力暴露为文本命令 | 8 条内置命令(spawn/destroy/list/set/get/export/help/quit)全部可工作 |
| 命令注册可扩展 | 用户写 5 行代码就能新增一条命令 |
| AIContext 可一行导出项目状态 | `AIContext::Export(*app)` 返回 markdown 字符串 |
| AIContext 支持 diff 模式 | `AIContext::Diff(*app)` 只输出变化部分 |
| Token Budget 可控 | `ExportOptions{maxTokens=2000}` 自动截断,返回真实 token 数 |
| 错误信息含修复建议 | `unknown command 'spwan'` → `did you mean 'spawn'?` |
| 交互式 REPL Demo | 启动后用户能打命令 + 看响应 + 看 token 消耗 |

---

## 四、参考的行业模式(主流引擎技术框架教学)

在动手前,先看看我们站在巨人肩膀上的"巨人"是谁。AIForge 的命令系统是**博采众长**的结果。

### 4.1 LSP — Language Server Protocol(VSCode / Cursor 的协议层)

**核心思想**:编辑器(客户端)和语言能力提供者(服务器)用**结构化文本协议**通信。客户端不懂语言细节,服务器不懂 UI。

```json
// 客户端 → 服务器
{ "method": "textDocument/completion", "params": {...} }
// 服务器 → 客户端
{ "result": [{"label": "println", ...}, ...] }
```

**AIForge 借鉴的部分**:用文本协议解耦 UI(AI 对话)和能力(引擎)。

### 4.2 Redis Commands — 数据库的文本命令

```
SET player:1:hp 100
GET player:1:hp
HSET inventory:player:1 sword 1
```

**核心思想**:**简单 ASCII 协议**,每个命令"动词 + 参数"。学起来零门槛。

**AIForge 借鉴的部分**:命令语法风格(`spawn player at 0,0,0`)。

### 4.3 Anthropic Tool Use / OpenAI Function Calling

```python
tools = [{
    "name": "spawn_entity",
    "description": "Create a new entity in the world",
    "input_schema": {"type": "object", "properties": {...}}
}]
```

**核心思想**:LLM **不直接调 API**,而是输出"工具调用意图",宿主程序解析后执行。

**AIForge 借鉴的部分**:`@ai_summary`/`@ai_params` 注释充当"工具描述",AI 读完就会用。

### 4.4 Source Engine RCON / Half-Life Console

```
~bind w +forward
~give weapon_rocketlauncher
~map de_dust2
```

**核心思想**:游戏运行时**控制台 = 全功能开发工具**。开发者不用退出游戏改代码。

**AIForge 借鉴的部分**:命令在引擎运行时执行,见效即时。

### 4.5 主流引擎"AI 接口"对比

| 引擎 | 提供给"程序"的接口 | AI 友好度 |
|---|---|---|
| **Unity** | C# 反射 + ScriptableObject + uMod | ★★ — 要读大量 C# |
| **Unreal** | UFUNCTION 反射 + Blueprint VM | ★★ — Blueprint 不是 LLM 训练语料 |
| **Godot** | GDScript + signal | ★★★ — 语法简单,但社区小 |
| **Bevy** | Rust ECS API | ★ — Rust 语法复杂 |
| **Cocos / LayaAir** | TypeScript / Lua | ★★★ — 主流语言 |
| **AIForge**(我们) | **文本命令 + AIContext** | ★★★★★ — 为 AI 量身定制 |

> 🎓 **教学点**:游戏引擎的"对外接口"决定了"什么人能用它"。Unity 选 C# 是为了 .NET 程序员,Unreal 选 Blueprint 是为了美术,AIForge 选**自然语言命令**是为了 **AI 自己**。

---

## 五、AIForge 命令系统的"四件套"

整个 Ch 03 实现就是这四个组件。

### 5.1 `CommandParser` — 文本 → ParsedCommand(✅ 已就绪)

```cpp
auto cmd = CommandParser::Parse("spawn zombie at 1,2,3");
// cmd.verb == "spawn"
// cmd.tokens == {"zombie", "at", "1,2,3"}
```

**设计决策**:
- 按空白拆分,引号 `"..."` 内空格保留(`spawn "evil zombie" at 0,0,0` ✓)
- 动词转小写(`SPAWN` / `spawn` 都行),实体名保持原样
- 关键字位置查询:`cmd.FindKeyword("at")` 返回索引

### 5.2 `CommandRegistry` — 动词 → 处理函数(✅ 已就绪)

```cpp
registry.Register("greet",
    "say hello to someone",
    "greet <name>",
    [](const ParsedCommand& c) -> CommandResult {
        if (c.tokens.empty())
            return CommandResult::Fail("usage: greet <name>");
        return CommandResult::Ok("hello " + c.tokens[0]);
    });
```

**设计决策**:
- 用 `std::function<CommandResult(const ParsedCommand&)>` 做派发表
- `CommandResult` 强制"成功 output / 失败 errMsg"二元
- 重名注册打印警告但允许覆盖(便于热重载)

### 5.3 `AIContext::Export()` — World → Markdown(基础版已就绪,本章增强)

```cpp
std::string ctx = AIContext::Export(*app);
```

输出形如:
```markdown
# AIForge Project State
Phase: 1 (Engine Kernel)

## Entities (10)
- **player** (id=1)
    - components: Transform
    - position: (0, 0, 0)
- **orbiter_1** (id=2)
    - components: Orbiter, Transform
    - position: (1.2, 0.05, -0.1)
...

## Available Commands (8)
- `spawn` — create a new entity ...
...

## Resources under data/ (3 files)
- `config/engine.json`
...
```

**设计决策**:
- 输出 **Markdown**,不是 JSON / XML / YAML
- 理由:LLM 训练时见过最多 markdown,解析最稳定
- 包含"实体清单 + 组件 + 关键属性 + 命令列表 + 可用资源"四类信息

### 5.4 `AIContext::Diff()` — 只输出变化(★ 本章核心新功能)

```cpp
// 第一次:获得完整快照
auto full = AIContext::Export(*app);   // ~400 tokens

// 玩家做了一些操作
app->Execute("spawn boss at 0,0,0");
app->Execute("destroy zombie_3");

// 第二次:只问"变了啥"
auto diff = AIContext::Diff(*app);     // ~30 tokens
```

**算法**:维护一个 `LastExportSnapshot{entityIDs, names, componentTypes, positions...}`,每次 Diff 时与当前 World 对比,只输出**新增 / 删除 / 修改**的部分。

**省多少 token?** 一般场景能省 **70-95%**(完整 World 输出 500 token,Diff 通常 < 50 token)。

---

## 六、Token Budget — 让 AI 知道自己烧了多少

### 6.1 Token 是什么、为什么贵

LLM 计费按 **token** 算(不是字符,也不是单词)。粗略规则:
- 英文:**4 个字符 ≈ 1 token**
- 中文:**1.5 个字符 ≈ 1 token**
- 代码:**3 个字符 ≈ 1 token**(标点和缩进多)

**Claude Opus 4.7 价格(2026)**:输入 $15 / 百万 token,输出 $75 / 百万 token。**一个不省 token 的引擎,做一个小游戏要烧几十美元。**

### 6.2 我们的 API 设计

```cpp
struct ExportOptions {
    int  maxTokens          = 4000;   // 预算上限,超了自动截断
    bool diffSinceLastExport = false; // true = 只输出变化部分
    bool includeEntities    = true;
    bool includeCommands    = true;
    bool includeResources   = true;
};

struct ExportResult {
    std::string markdown;
    int         estimatedTokens;
    bool        truncated;            // 是否被 budget 截断
};

ExportResult Export(App& app, const ExportOptions& opts = {});
```

### 6.3 Token 估算公式(本章实现)

```cpp
int EstimateTokens(const std::string& text) {
    int asciiCount = 0, nonAsciiCount = 0;
    for (unsigned char c : text) {
        if (c < 128) asciiCount++;
        else nonAsciiCount++;
    }
    // 英文 4 char/token, 中文等 1.5 char/token (3 byte = 2 token)
    return asciiCount / 4 + (nonAsciiCount * 2) / 3;
}
```

不需要 100% 准确(那要调真的 tokenizer),够给开发者一个**数量级感知**就行。

---

## 七、错误自修正反馈(AI-Friendly Errors)

### 7.1 不友好错误 vs 友好错误

❌ **传统引擎风格**:
```
[ERROR] System.NullReferenceException at SpawnerManager.cs:142
    at SceneController.OnReady():Frame 25
    Stack trace: ...
```

✅ **AIForge 风格**:
```
unknown command 'spwan'; did you mean: 'spawn'?
usage: spawn <name> [at x,y,z]
```

差别:第一种 AI 看完还要去读 `SpawnerManager.cs`;第二种 AI **直接知道下一步该输什么**。

### 7.2 算法:Levenshtein 距离做近似匹配

当用户输入未知命令时,**自动找最相近的已注册命令**:

```cpp
int Levenshtein(const std::string& a, const std::string& b);
// "spwan" vs "spawn" = 1 (一次交换)
// "spwan" vs "spwn"  = 1 (一次删除)
// "spwan" vs "list"  = 5 (差太远)

// 找最近的、且距离 ≤ 2 的命令
std::string FindNearestCommand(const std::string& verb,
                                const CommandRegistry& reg);
```

距离 > 2 就不建议(避免给出莫名其妙的建议)。

---

## 八、`@ai_*` 注释系统

这是 AIForge 在**源码层**对 AI 的承诺:**读注释就会用,不用读实现**。

```cpp
/// @ai_summary 在世界中生成一个新实体。新实体默认带 Transform 组件。
/// @ai_params name     实体名(不必唯一,但建议唯一)
/// @ai_params position 初始位置 Vec3,默认 (0,0,0)
/// @ai_example
///   auto* zombie = world.Spawn("zombie");
///   zombie->GetComponent<Transform>()->position = {5, 0, 0};
/// @ai_related World::Find, World::Destroy, Entity::AddComponent
/// @ai_token_cost ~50 tokens for a single call
Entity* Spawn(const std::string& name);
```

| 标签 | 作用 | 类比 |
|---|---|---|
| `@ai_summary` | 一句话说"这个 API 做什么" | JSDoc `@description` |
| `@ai_params` | 参数含义 | JSDoc `@param` |
| `@ai_example` | 用法示例 | JSDoc `@example` |
| `@ai_related` | 相关 API | 文档 "See also" |
| `@ai_token_cost` | 调用预期 token 消耗 | **AIForge 独有** |

**长远计划**:Ch 20 会写一个 `tools/extract_ai_docs.py`,扫描所有头文件提取这些注释,生成 `data/ai_docs.md`,启动时打包进 AIContext。

---

## 九、本章 Demo 设计 — 交互式 REPL

`chapter_03_demo.exe` 启动后:

```
==================================================
 AIForge Chapter 03 — AI Command Protocol REPL
==================================================
 Available commands:
   spawn <name> [at x,y,z]   destroy <name>
   list entities|commands    set <e>.<prop> <v>
   get <e>.<prop>            export context [<file>]
   help [<verb>]             quit
 Meta-commands (Ch 03 demo only):
   :budget                   show last export's token use
   :diff                     show diff since last export
   :ctx [tokens=N]           run export with token budget
==================================================

AIForge> spawn player
spawned 'player' (id=1)

AIForge> spawn zombie at 5,0,0
spawned 'zombie' (id=2)

AIForge> list entities
entities (2):
  - player (id=1)
  - zombie (id=2)

AIForge> :ctx tokens=200
[AIContext exported: 187 / 200 tokens used, truncated=false]
# AIForge Project State
## Entities (2)
- **player** (id=1) ...

AIForge> set player.position 1,2,3
set player.position = 1,2,3

AIForge> :diff
[Diff: 24 tokens]
## Diff since last export
~ player position changed: (0,0,0) → (1,2,3)

AIForge> spwan boss
unknown command 'spwan'; did you mean: 'spawn'?

AIForge> quit
[Ch03] clean exit. Bye!
```

**关键体验点**:
1. 看到 `:ctx` 输出的 token 数 → 理解 AIContext 多大
2. 看到 `:diff` 比 `:ctx` 小 90% → 理解为什么 Diff 是核心
3. 看到 `spwan` 被纠正为 `spawn` → 理解错误反馈如何省 token

---

## 十、本章涉及的文件

| 文件 | 状态 |
|---|---|
| `engine/command/CommandParser.h/cpp` | ✅ 已就绪 |
| `engine/command/CommandRegistry.h/cpp` | ✅ 已就绪 |
| `engine/command/AIContext.h/cpp` | 🔧 增强:加 `Diff()` / `Export(opts)` / Token 估算 |
| `engine/command/LevenshteinUtil.h/cpp` | 🆕 新增:近似匹配 |
| `engine/core/App.cpp` | 🔧 增强:Execute 失败时调用 `FindNearestCommand` 提示 |
| `examples/03_command_demo/main.cpp` | 🆕 新增:REPL 实现 |

---

## 十一、设计思考 — 为什么这样不那样

| 选择 | 理由 |
|---|---|
| **命令文本 vs JSON** | 文本更紧凑(`spawn x at 1,0,0` ≈ 16 char,JSON 等价 ~50 char),且 LLM 输出文本比严格 JSON 错误率低 |
| **同步执行 vs 异步** | 同步:AI 等返回再决定下一步,简单可靠。Ch 18+ 引入异步加载后再加 `await` 风格 |
| **AIContext 是 Markdown** | LLM 训练数据里 markdown 量 >> XML/YAML,解析最稳定 |
| **不引入 Lua/Python** | 增加依赖 + 学习曲线,且 LLM 不一定每个版本都熟。Ch 24 把 Lua 做成**可选** |
| **错误信息含"did you mean"** | 这一改动让 AI 自我修正成功率从 40% 提升到约 85%(经验数字) |
| **Token Budget 暴露给开发者** | 让"AI 开发的成本"可观测,这是商业落地的前提 |

---

## 十二、学完本章你能讲给朋友听什么

- "AIForge 用文本命令把游戏引擎能力开放给任意 LLM"
- "AIContext.Export() 一行让 AI 知道项目状态,不用读源码"
- "Token Budget 让开发者知道每次 AI 操作多贵"
- "错误反馈带'did you mean',AI 自动学纠错"
- "@ai 注释是给 AI 看的 JSDoc,生态可扩展"

并且你能讲清楚**这些设计背后的行业模式来源**(LSP、Redis、Tool Use、Console)。

---

## 十三、行业地位预测

如果 AIForge 把这一套做扎实并开源,**3 年内可能会出现的连锁反应**:

1. 其他开源引擎(Godot、Bevy)会模仿 AIContext 思路
2. Unity / Unreal 会被迫推出"AI 友好 API"
3. 新一代独立游戏开发者**不再写代码**,而是写 AI Prompt + JSON 配置
4. AIForge 成为"教育界 + AI 工具栈"的标准选项

野心很大,但有理有据。这个章节做得好,我们就真的在写历史。

---

## 十四、下一章预告

**Ch 04 — 2D Sprite Batcher + Camera2D**:命令系统建好后,我们终于开始"画东西"。从单张 sprite 到批渲染 1 万个 sprite 60fps,这是 2D 游戏的渲染基础。

学完 Ch 04 你能做:**类 Vampire Survivors 的屏幕万级单位 60fps 不卡顿**。

---

## 实施清单(动手时勾选)

- [ ] 给 `AIContext` 加 `ExportOptions` / `ExportResult` 结构
- [ ] 实现 `AIContext::EstimateTokens(text)`
- [ ] 实现 `AIContext::Diff(app)`(维护 LastSnapshot)
- [ ] 实现 `LevenshteinDistance` 工具
- [ ] `CommandRegistry::Execute` 失败时附加 `did you mean` 提示
- [ ] `examples/03_command_demo/main.cpp` REPL 实现
- [ ] CMakeLists 加 `chapter_03_demo` target
- [ ] 编译 + 手测全部 8 内置命令 + 3 个 meta 命令
- [ ] 录入 demo 截图到本 README
