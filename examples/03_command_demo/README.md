# Chapter 03 — AI 命令协议 + Context 系统(★ 引擎核心创新)

> **本章定位**:AIForge 的"心脏"。这一章定义了"AI 怎么用引擎"。

---

## 本章目标
1. **CommandParser**:把 `"spawn zombie at 5,0,0"` 解析成 `{verb, tokens}`
2. **CommandRegistry**:动词 → 处理函数 的注册表,内置 8 条命令(spawn / destroy / list / set / get / export / help / quit)
3. **AIContext.Export()**:一行调用,导出"项目当前状态 markdown"
4. **AIContext.Diff()**:仅导出"上次导出后变化的部分"(★ 省 token 关键)
5. **Token Budget**:Export 可指定 `maxTokens`,自动截断 + 优先级排序
6. **错误自修正**:失败信息含"为什么 + 怎么改"

## 跑出来的 Demo
`build/Release/chapter_03_demo.exe` — 一个交互式 REPL:
```
AIForge> spawn player at 0,0,0
spawned 'player' (id=1)
AIForge> spawn zombie at 5,0,0
spawned 'zombie' (id=2)
AIForge> list entities
entities (2):
  - player (id=1)
  - zombie (id=2)
AIForge> set player.position 1,2,3
set player.position = 1,2,3
AIForge> export context
# AIForge Project State
...
AIForge> token budget                ← ★ 独家
last export: 412 tokens (limit 2000)
diff next call: ~80 tokens (-80% saving)
AIForge> quit
```

## 学到的前沿技术
- **文本协议设计**:类似 Redis CLI / Git 命令的"动词 + 参数"结构,AI 友好
- **`std::function` 派发表**:类型擦除 + 闭包捕获 → 优雅的命令分发
- **Token 计数估算**:用 OpenAI 的"4 字符 ≈ 1 token"快速规则做预算
- **状态 Diff 算法**:对比快照,输出"新增/删除/修改"的最小集
- **AI 友好错误**:`unknown command 'spwan'; did you mean: 'spawn'?`(Levenshtein 距离 ≤ 2 的近似匹配)

## 背景知识(为什么这章必要)

这是 AIForge 与所有其他引擎的**本质差异**。

**Unity / Unreal 的工作流**:AI 看 C# 代码 → 生成新 C# 代码 → IDE 编译 → 报错 → AI 看错误堆栈 → 改代码。**单次任务 5000-20000 token,容易死循环**。

**AIForge 的工作流**:AI 发命令 → 引擎执行 → 引擎反馈"成功" 或 "失败 + 怎么改" → AI 决定下一步。**单次任务 200-1000 token,失败可立刻自修正**。

为什么省这么多?
1. AI 不需要读源码,只读 `@ai_summary` + 命令列表
2. AIContext.Diff 只输出变化,不输出全量
3. 错误信息直接告诉 AI 怎么改,不需要 AI 推理
4. 命令是文本,不需要 AI 处理 C++ 语法

## 架构设计

```
用户/AI 输入文本
   ↓
CommandParser::Parse  → ParsedCommand{verb, tokens, raw}
   ↓
CommandRegistry::Execute → 查表 → handler(parsed)
   ↓
CommandResult{ok, output, errMsg}
   ↓
返回给用户/AI

────────────────────────────────────

AIContext::Export(app, opts)
   1. 序列化 World(实体 + 组件 + 关键属性)
   2. 列出可用命令(按 budget 截断)
   3. 列出可用资源(按 budget 截断)
   4. 计算 token 数,记录到 LastExport 缓存
   ↓
返回 markdown string

AIContext::Diff(app)
   1. 取 LastExport 快照
   2. 对比当前 World
   3. 仅输出"变化的实体/组件"
   ↓
返回 markdown diff(可能 < 100 tokens)
```

## 涉及源文件
- [`engine/command/CommandParser.h/cpp`](../../engine/command/CommandParser.h)
- [`engine/command/CommandRegistry.h/cpp`](../../engine/command/CommandRegistry.h)
- [`engine/command/AIContext.h/cpp`](../../engine/command/AIContext.h)
- [`tools/ai_context_export/export.cpp`](../../tools/ai_context_export/export.cpp) — 命令行工具
- [`templates/new_entity.json`](../../templates/new_entity.json) — AI 任务模板

## Token Budget API(本章独家落地)

```cpp
namespace AIForge {
struct ExportOptions {
    int maxTokens = 4000;
    bool diffSinceLastExport = false;
    bool includeResources = true;
    bool includeCommands = true;
};

class AIContext {
public:
    struct Result {
        std::string markdown;
        int estimatedTokens;
        bool truncated;
    };

    static Result Export(const App& app, const ExportOptions& opts = {});
    static Result Diff(const App& app);
    static int    EstimateTokens(const std::string& text);  // 4-char rule
};
}
```

## 8 条内置命令(完整规格)
| 命令 | 用法 | 失败示例 |
|---|---|---|
| `spawn` | `spawn <name> [at x,y,z]` | `usage: spawn <name> [at x,y,z]` |
| `destroy` | `destroy <name>` | `no entity named 'foo'. Try 'list entities'.` |
| `list` | `list entities \| list commands` | `usage: list entities \| list commands` |
| `set` | `set <entity>.<prop> <value>` | `unknown property 'pos'; did you mean: 'position'?` |
| `get` | `get <entity>.<prop>` | 同上 |
| `export` | `export context [<filepath>]` | `cannot open output file '/x'` |
| `help` | `help [<verb>]` | `no such command: 'fooo'` |
| `quit` | `quit` | (永远成功) |

## 实施清单
- [x] ParsedCommand 结构 + tokenize(支持引号)
- [x] CommandRegistry::Register / Execute / List / Find
- [x] 8 条内置命令实现
- [x] AIContext::Export 基础版
- [ ] AIContext::Diff(★ 本章重点)
- [ ] Token 估算 + maxTokens 截断
- [ ] 错误近似匹配("did you mean...")
- [ ] chapter_03_demo REPL
- [ ] 多模型示例脚本

## 课后练习
1. 加一条新命令 `clone <src> <dst>`(复制实体连组件一起)
2. 让 `set` 支持 `+=` / `-=` 操作(`set player.position += 1,0,0`)
3. 写一个 Python 脚本,把 OpenAI API 的输出当命令喂给本 demo,跑一个"AI 自主搭场景"

## 常见坑
- 引号内空格不要 split:`spawn "evil zombie" at 0,0,0`
- 命令大小写:动词转小写,但实体名保持大小写敏感
- Token 估算的 4-char 规则只适用于英文 + ASCII;中文是 1.5-2 字符 / token,要分别处理

## 延伸阅读
- [Anthropic — Tool Use Best Practices](https://docs.anthropic.com/claude/docs/tool-use) — Claude 怎么调外部工具
- [OpenAI — Function Calling Guide](https://platform.openai.com/docs/guides/function-calling)
- [LSP Protocol](https://microsoft.github.io/language-server-protocol/) — VSCode/Cursor 用的协议,和我们的命令系统设计哲学相通
- [Game Programming Patterns — Command Pattern](https://gameprogrammingpatterns.com/command.html)

## 下一章预告
**Ch 04 — 2D Sprite Batcher**:引擎心脏跳起来后,我们开始在屏幕上画东西。从最经典的 2D sprite 渲染开始,目标是单 draw call 1 万 sprite。
