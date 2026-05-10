# AIForge Engine — Claude Code 执行指令

## 项目概述
这是一个开源的AI-First游戏引擎框架（AIForge Engine），专为AI模型高效开发游戏设计。核心创新：统一命令接口、自描述API(@ai注释)、AI上下文自动导出、数据驱动架构。

## 请先完整读取以下文件
1. README.md — 项目总览和架构
2. PHASES.md — 各阶段详细规范

## 当前任务：Phase 1 引擎内核

请严格按照 PHASES.md 中 Phase 1 的规范创建所有文件。

### 技术要求

1. C++17标准
2. SDL3通过CMake FetchContent自动下载（参考PHASES.md中的CMake配置）
3. OpenGL 4.5（Phase 1只初始化，不做复杂渲染）
4. 所有头文件必须有 @ai_summary/@ai_params/@ai_example/@ai_related 注释
5. 错误处理：所有函数失败时打印可读错误信息并返回nullptr/false，不要抛异常不要崩溃
6. 内存管理：使用智能指针(unique_ptr/shared_ptr)，不要裸new/delete

### Phase 1 需要创建的完整文件列表

```
AIForge/
├── engine/
│   ├── core/
│   │   ├── App.h          — 应用程序类（Init/Run/Shutdown生命周期）
│   │   ├── App.cpp
│   │   ├── Window.h        — SDL3窗口封装（创建/销毁/事件处理）
│   │   ├── Window.cpp
│   │   ├── Input.h         — 统一输入系统（键盘/鼠标/手柄，SDL3事件驱动）
│   │   ├── Input.cpp
│   │   ├── Time.h          — 时间管理（deltaTime/FPS计算/固定步长）
│   │   ├── Time.cpp
│   │   ├── ECS.h           — 实体组件系统（Entity/Component/World）
│   │   ├── ECS.cpp
│   │   ├── ResourceManager.h — 资源管理（路径解析/加载/缓存/引用计数）
│   │   └── ResourceManager.cpp
│   ├── command/
│   │   ├── CommandParser.h  — 文本命令解析（"spawn player at 0,0,0"）
│   │   ├── CommandParser.cpp
│   │   ├── CommandRegistry.h — 命令注册表（各系统注册自己支持的命令）
│   │   ├── CommandRegistry.cpp
│   │   ├── AIContext.h      — AI上下文导出（自动生成项目状态描述）
│   │   └── AIContext.cpp
│   └── scene/
│       ├── SceneLoader.h    — JSON场景加载器
│       └── SceneLoader.cpp
├── data/
│   └── config/
│       └── engine.json      — 引擎默认配置（窗口大小/标题/FPS等）
├── templates/
│   └── new_entity.json      — AI任务模板：创建新实体
├── tools/
│   └── ai_context_export/
│       └── export.cpp       — 命令行工具：导出AI上下文到文件
├── examples/
│   ├── 01_hello_window/
│   │   ├── main.cpp         — 示例：创建SDL3窗口+基础渲染循环
│   │   └── README.md
│   ├── 02_ecs_demo/
│   │   ├── main.cpp         — 示例：ECS创建实体+添加组件+查询
│   │   └── README.md
│   └── 03_command_demo/
│       ├── main.cpp         — 示例：文本命令驱动引擎
│       └── README.md
├── docs/
│   ├── AI_GUIDE.md          — AI开发者指南（如何用命令接口）
│   └── ARCHITECTURE.md      — 架构说明
├── CMakeLists.txt            — 顶层构建（SDL3 FetchContent + 所有target）
├── README.md                 — 项目说明（已提供，放到项目根目录）
├── PHASES.md                 — 阶段规范（已提供，放到项目根目录）
└── LICENSE                   — MIT协议
```

### ECS核心实现要求

```cpp
// Component基类
struct Component {
    virtual ~Component() = default;
    virtual const char* GetType() const = 0;
};

// 位置组件
struct Transform : Component {
    Vec3 position = {0, 0, 0};
    Vec3 rotation = {0, 0, 0};  // 欧拉角（度）
    Vec3 scale = {1, 1, 1};
    const char* GetType() const override { return "Transform"; }
};

// Entity必须有的方法
class Entity {
public:
    uint32_t GetID() const;
    const std::string& GetName() const;
    bool IsActive() const;
    void SetActive(bool active);
    
    template<typename T, typename... Args> T* AddComponent(Args&&... args);
    template<typename T> T* GetComponent();
    template<typename T> bool HasComponent();
    void RemoveComponent(const std::string& type);
};

// World必须有的方法
class World {
public:
    Entity* Spawn(const std::string& name);
    Entity* Find(const std::string& name);
    Entity* FindByID(uint32_t id);
    std::vector<Entity*> FindByComponent(const std::string& type);
    void Destroy(Entity* entity);
    void DestroyAll();
    void Update(float dt);
    int GetEntityCount() const;
};
```

### 命令系统核心要求

```cpp
// 命令格式："动词 参数1 参数2 ..."
// 例子：
//   "spawn player at 0,0,0"
//   "destroy player"
//   "set player.position 10,0,5"
//   "list entities"
//   "export context"

// 必须支持的内置命令（Phase 1）：
// spawn <name>                — 创建实体
// spawn <name> at <x,y,z>     — 创建实体并设置位置
// destroy <name>              — 销毁实体
// list entities               — 列出所有实体
// set <entity.property> <value> — 设置属性
// get <entity.property>       — 获取属性
// export context              — 导出AI上下文
// help                        — 列出所有可用命令
```

### JSON配置格式

engine.json:
```json
{
    "window": {
        "title": "AIForge App",
        "width": 1280,
        "height": 720,
        "fullscreen": false,
        "vsync": true
    },
    "engine": {
        "targetFPS": 60,
        "fixedTimeStep": 0.016,
        "maxDeltaTime": 0.1
    }
}
```

### 示例程序要求

01_hello_window: 创建SDL3窗口，显示蓝色背景，ESC退出，窗口标题显示FPS
02_ecs_demo: 创建World，生成10个实体，添加Transform组件，每帧更新位置，控制台打印实体信息
03_command_demo: 启动命令循环，用户输入命令操作实体（spawn/destroy/list/set/get），展示命令系统

### 每个示例的README.md要求
1. 本示例实现了什么
2. 新增的API（带@ai注释原文）
3. 运行方式
4. AI使用指南

### 验收标准
1. cmake编译通过，3个示例全部能运行
2. 所有头文件都有@ai注释
3. 命令系统能解析和执行基础命令
4. AIContext能导出当前世界状态为可读文本
5. engine.json能被正确加载
