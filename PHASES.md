# AIForge Engine — 阶段开发规范

## 总原则

1. 每个阶段独立可编译可运行
2. 每个阶段有示例程序验证功能
3. 每个头文件必须有 @ai_summary/@ai_params/@ai_example 注释
4. 所有系统通过抽象接口解耦
5. 游戏逻辑全部数据驱动（JSON配置）
6. 每个阶段更新README.md

---

## Phase 1：引擎内核（最重要，基础决定一切）

### 目标
搭建引擎骨架：SDL3窗口、输入系统、ECS实体组件系统、命令解析器、JSON加载、AI上下文导出。这个阶段不做渲染（只有窗口+色块），专注架构。

### 需要创建的文件

```
engine/core/App.h/cpp              — 应用生命周期（Init/Run/Shutdown）
engine/core/Window.h/cpp           — SDL3窗口封装
engine/core/Input.h/cpp            — 输入系统（键盘/鼠标/手柄统一接口）
engine/core/Time.h/cpp             — deltaTime/FPS/固定时间步
engine/core/ECS.h                  — 实体组件系统（轻量级）
engine/core/ResourceManager.h      — 资源加载与缓存（路径解析+引用计数）
engine/command/CommandParser.h/cpp  — 文本命令解析器
engine/command/CommandRegistry.h    — 命令注册表（引擎各系统注册自己的命令）
engine/command/AIContext.h/cpp      — AI上下文导出器（导出项目状态给AI）
engine/scene/SceneLoader.h         — JSON场景加载器
data/config/engine.json            — 引擎默认配置
templates/new_entity.json          — AI任务模板：创建实体
tools/ai_context_export/export.cpp — 上下文导出命令行工具
examples/01_hello_window/main.cpp  — 示例：SDL3窗口
examples/02_ecs_demo/main.cpp      — 示例：ECS系统
examples/03_command_demo/main.cpp  — 示例：命令系统
docs/AI_GUIDE.md                   — AI开发者指南
CMakeLists.txt                     — 构建配置（SDL3集成）
README.md                          — 项目说明
```

### ECS设计核心

```cpp
// 轻量级ECS — AI好理解，不用模板元编程
class Entity {
    uint32_t id;
    std::string name;
    std::unordered_map<std::string, Component*> components;
public:
    template<typename T, typename... Args>
    T* AddComponent(Args&&... args);
    
    template<typename T>
    T* GetComponent();
    
    void RemoveComponent(const std::string& type);
};

class World {
    std::vector<Entity*> entities;
public:
    Entity* Spawn(const std::string& name);
    Entity* Find(const std::string& name);
    void Destroy(Entity* e);
    void Update(float dt);
    
    // AI友好：按类型查询
    std::vector<Entity*> FindByComponent(const std::string& componentType);
};
```

### 命令系统设计

```cpp
// 引擎各系统注册命令
CommandRegistry::Register("spawn", [](Args args) {
    // spawn entity_name at x,y,z
    world->Spawn(args["name"])->SetPosition(args["pos"]);
});

CommandRegistry::Register("destroy", [](Args args) {
    world->Destroy(world->Find(args["name"]));
});

// AI或人类执行命令
engine.Execute("spawn zombie at 10,0,5");
engine.Execute("destroy zombie");
```

### AI上下文导出器

```cpp
// 自动生成项目当前状态的精简描述
class AIContext {
public:
    static std::string Export(World* world) {
        std::string ctx;
        ctx += "# 当前项目状态\n";
        ctx += "## 实体列表\n";
        for (auto e : world->entities) {
            ctx += "- " + e->name + " [";
            for (auto& [type, comp] : e->components)
                ctx += type + ",";
            ctx += "]\n";
        }
        ctx += "## 可用命令\n";
        for (auto& cmd : CommandRegistry::GetAll())
            ctx += "- " + cmd.name + ": " + cmd.description + "\n";
        ctx += "## 可用资源\n";
        // 列出data/目录下所有资源文件
        return ctx;
    }
};
```

### SDL3集成要点

```cmake
# CMakeLists.txt
# SDL3可以通过FetchContent自动下载
include(FetchContent)
FetchContent_Declare(SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.0)
FetchContent_MakeAvailable(SDL3)
target_link_libraries(${PROJECT_NAME} SDL3::SDL3)
```

### @ai注释规范

```cpp
/// @ai_summary 在世界中生成一个新实体
/// @ai_params name: 实体名称（唯一）
/// @ai_params pos: 初始位置 Vec3，默认(0,0,0)
/// @ai_example auto player = world->Spawn("player");
/// @ai_example player->SetPosition({0, 1, 0});
/// @ai_related World::Find, World::Destroy, Entity::AddComponent
Entity* Spawn(const std::string& name);
```

---

## Phase 2：渲染与动画

### 目标
OpenGL渲染抽象层、2D/3D渲染、材质系统、骨骼动画、天空盒、粒子系统。

### 需要创建的文件

```
engine/render/RenderAPI.h           — 渲染抽象接口
engine/render/backend_gl/GLRender.h/cpp — OpenGL实现
engine/render/Shader.h/cpp         — Shader管理
engine/render/Mesh.h/cpp           — 网格数据+工厂方法
engine/render/Material.h           — 材质（漫反射/法线/粗糙度贴图）
engine/render/Texture.h/cpp        — 纹理加载
engine/render/Camera.h             — 摄像机（Orbit/FPS/TPS）
engine/render/Skybox.h             — 天空盒
engine/render/ParticleSystem.h     — 粒子系统
engine/render/ModelLoader.h        — Assimp模型加载
engine/animation/Animator.h        — 骨骼动画
engine/animation/AnimStateMachine.h — 动画状态机
data/shaders/standard.vs/fs        — 标准PBR着色器
data/shaders/skinning.vs/fs        — 蒙皮着色器
data/shaders/skybox.vs/fs          — 天空盒着色器
data/shaders/particle.vs/fs        — 粒子着色器
examples/04_render_3d/main.cpp     — 示例：3D渲染
examples/05_animation/main.cpp     — 示例：骨骼动画
```

### 渲染抽象层设计

```cpp
// 抽象接口 — 后续添加Vulkan只需实现这个接口
class RenderAPI {
public:
    virtual void Init(Window* window) = 0;
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void DrawMesh(Mesh* mesh, Material* mat, const Mat4& transform) = 0;
    virtual void DrawSkybox(Skybox* sky, Camera* cam) = 0;
    virtual ShaderHandle LoadShader(const char* vs, const char* fs) = 0;
    virtual TextureHandle LoadTexture(const char* path) = 0;
};

// 创建后端
RenderAPI* CreateOpenGLRenderer();  // Phase 2
RenderAPI* CreateVulkanRenderer();  // Phase 5
```

---

## Phase 3：物理与音频

### 目标
PhysX物理封装、FMOD音频封装、碰撞事件系统、3D空间音效。

### 需要创建的文件

```
engine/physics/PhysicsAPI.h         — 物理抽象接口
engine/physics/backend_physx/PhysXWorld.h/cpp — PhysX实现
engine/physics/CollisionEvent.h     — 碰撞事件
engine/audio/AudioAPI.h             — 音频抽象接口
engine/audio/backend_fmod/FMODAudio.h/cpp — FMOD实现
examples/06_physics/main.cpp        — 示例：物理碰撞
examples/07_audio/main.cpp          — 示例：3D音效
templates/new_physics_object.json   — AI模板：物理对象
```

### 物理抽象层

```cpp
class PhysicsAPI {
public:
    virtual void Init(Vec3 gravity = {0, -9.81f, 0}) = 0;
    virtual void Step(float dt) = 0;
    virtual void AddStaticBox(Vec3 pos, Vec3 halfSize) = 0;
    virtual void AddDynamicBox(Vec3 pos, Vec3 halfSize, float mass) = 0;
    virtual void AddGroundPlane() = 0;
    virtual bool Raycast(Vec3 origin, Vec3 dir, float maxDist, HitResult& out) = 0;
    virtual void SetCollisionCallback(CollisionCallback cb) = 0;
};
```

---

## Phase 4：游戏系统

### 目标
战斗系统、AI行为树、场景管理、游戏UI、存档系统。这个阶段让引擎能做完整游戏。

### 需要创建的文件

```
engine/gameplay/CombatSystem.h      — 战斗（攻击/受击/HitStop）
engine/gameplay/HealthComponent.h   — 生命值组件
engine/gameplay/WeaponComponent.h   — 武器组件
engine/ai/BehaviorTree.h            — 行为树
engine/ai/Pathfinding.h             — A*寻路
engine/scene/SceneManager.h         — 场景管理+切换
engine/scene/TriggerSystem.h        — 触发器
engine/ui/GameUI.h                  — 游戏UI（血条/飘字/HUD）
engine/ui/DebugUI.h                 — ImGui调试面板
engine/core/SaveSystem.h            — 存档/读档（JSON）
templates/new_enemy.json            — AI模板
templates/new_weapon.json           — AI模板
templates/new_level.json            — AI模板
templates/new_skill.json            — AI模板
examples/08_combat/main.cpp         — 示例：战斗系统
examples/09_ai_demo/main.cpp        — 示例：敌人AI
```

---

## Phase 5：高级特性

### 目标
Vulkan渲染后端、阴影映射、后处理效果、Lua脚本、可视化编辑器。

### 需要创建的文件

```
engine/render/backend_vk/VKRender.h/cpp — Vulkan实现
engine/render/ShadowMap.h           — 阴影映射
engine/render/PostProcess.h         — 后处理（Bloom/HDR/FXAA）
engine/scripting/LuaEngine.h        — Lua脚本引擎（可选）
engine/editor/Editor.h              — 可视化编辑器（ImGui）
engine/editor/SceneHierarchy.h      — 场景层级面板
engine/editor/Inspector.h           — 属性检查器
engine/editor/AssetBrowser.h        — 资源浏览器
examples/10_shadows/main.cpp
examples/11_postprocess/main.cpp
examples/12_editor/main.cpp
```

---

## Phase 6：Demo游戏

### 目标
用AIForge引擎做一个完整3D动作游戏Demo，验证引擎能力，同时作为展示案例。

### 需要创建的文件

```
game/main.cpp                       — 游戏入口
game/entities/Player.json            — 玩家配置
game/entities/Zombie.json            — 僵尸配置
game/entities/Mutant.json            — BOSS配置
game/levels/level1_tutorial.json     — 教学关
game/levels/level2_arena.json        — 战斗关
game/levels/level3_boss.json         — BOSS关
game/config/weapons.json             — 武器配置
game/config/skills.json              — 技能配置
game/config/audio.json               — 音频配置
```

### Demo游戏的main.cpp应该很简洁

```cpp
#include "AIForge.h"

int main() {
    auto app = AIForge::CreateApp("config/game.json");
    app->LoadLevel("levels/level1_tutorial.json");
    app->Run();
    return 0;
}
```

所有游戏逻辑都在JSON配置中，C++代码极少。这就是AI-First的威力——AI生成JSON比写C++快10倍。

---

## 每个阶段的README模板

每个Phase完成后，在examples/对应目录下写README.md，包含：
1. 本阶段实现了什么
2. 新增API列表（带@ai注释）
3. 示例代码
4. AI使用指南（AI怎么用这些API）
5. 架构图
6. 下一阶段预告
