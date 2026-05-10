# Chapter 02 — ECS + Time + Input

> **本章定位**:游戏世界的"骨架" — 实体 / 组件 / 时间 / 输入。

---

## 本章目标
1. 实现轻量级 ECS(Entity / Component / World)
2. Time 系统:`deltaTime` / 总时长 / FPS / 固定步长
3. Input 系统:键盘 / 鼠标 / 手柄统一接口,提供"按下/按住/释放"三态查询
4. 在控制台打印 10 个旋转中实体的位置,WASD 控制其中一个

## 跑出来的 Demo
`build/Release/chapter_02_demo.exe` — 控制台每秒打印 10 个实体的位置变化,WASD 移动 entity_0,ESC 退出。

## 学到的前沿技术
- **ECS 架构**:为什么现代引擎都从"面向对象继承"转向"实体+组件" — Mike Acton 的 [Data-Oriented Design](https://www.youtube.com/watch?v=rX0ItVEVjHc) 思想
- **DOD(数据导向设计)**入门:CPU 缓存友好的数据布局
- **智能指针所有权**:`unique_ptr` 用作"独占资源"、`shared_ptr` 用作"共享资源"的取舍
- **帧无关固定步长**:为什么物理模拟必须用固定 dt(`Glenn Fiedler 经典文章`)
- **输入边缘 vs 电平触发**:`IsKeyPressed`(刚按下)和 `IsKeyDown`(按住)的区别

## 背景知识(为什么这章必要)
传统 OOP 引擎把 `Player` 当一个继承自 `Character` 继承自 `Actor` 的类,功能堆叠在继承树上。**问题**:加新功能要改基类,缓存命中差,组合困难。

ECS 把"是什么"(Entity 只有 ID + 名字)和"做什么"(Component 是数据,System 是逻辑)分开:
- **Entity** = ID + 名字 + 组件容器
- **Component** = 纯数据(位置 / 血量 / 武器)
- **System** = 处理特定组件的逻辑(渲染系统处理所有 RenderComponent)

Unity 在 2018 引入 DOTS,Unreal 在 5.0 引入 Mass Entity,Bevy 整个引擎都是 ECS。**这是行业不可逆趋势**。

我们的实现是"轻量 ECS"——AI 容易理解,不用模板元编程。重型 ECS(EnTT)留到性能瓶颈出现时再升级。

## 架构设计

```
World
 └─ vector<unique_ptr<Entity>>
       └─ Entity (id, name, active)
             └─ unordered_map<TypeName, unique_ptr<Component>>
                   ├─ Transform(position, rotation, scale)  ← 默认就有
                   ├─ Health (Ch 12 加)
                   ├─ Mesh   (Ch 07 加)
                   └─ ...

Time
 └─ Tick() → 计算 dt + 累积总时间 + 平滑 FPS

Input (事件驱动)
 └─ NewFrame() → 清"刚按下/释放"集合,保留"按住"集合
     OnKeyDown/Up/MouseMotion → 由 App 分发
     IsKeyDown/Pressed/Released → 查询
```

## 涉及源文件
- [`engine/core/ECS.h/cpp`](../../engine/core/ECS.h) — Entity / Component / World
- [`engine/core/Time.h/cpp`](../../engine/core/Time.h) — 时间管理
- [`engine/core/Input.h/cpp`](../../engine/core/Input.h) — 输入系统
- [`engine/core/Math.h/cpp`](../../engine/core/Math.h) — Vec2/3/4 + ParseVec3

## 实施清单
- [x] Component 基类 + Transform 默认组件
- [x] Entity::AddComponent / GetComponent / HasComponent / RemoveComponent
- [x] World::Spawn / Find / FindByID / FindByComponent / Destroy
- [x] Time::Tick + 平滑 FPS 计算
- [x] Input 三态查询
- [ ] chapter_02_demo target
- [ ] WASD 移动演示
- [ ] 截图录入

## 课后练习(HOMEWORK)
1. 给 Entity 加一个 `Tag`(string)字段,World 加 `FindByTag`
2. 实现 `Component::Update(dt)` 虚函数,让 World::Update 自动驱动
3. 写个 `Velocity` 组件,System 模式写一个"按 velocity 移动 transform"的更新

## 常见坑
- 在迭代 `world.GetEntities()` 时不要 `Destroy`(会失效迭代器)
- Component 的虚析构必须有,否则 unique_ptr 释放派生类会内存泄漏
- 输入事件如果忘了 `NewFrame()`,`IsKeyPressed` 会一直为 true

## 延伸阅读
- [Glenn Fiedler — Fix Your Timestep](https://gafferongames.com/post/fix_your_timestep/) — 物理时间步圣经
- [Sander Mertens — Building an ECS](https://ajmmertens.medium.com/building-an-ecs-1-where-are-my-entities-and-components-63d07c7da742) — flecs 作者的 ECS 系列
- [GDC — Data-Oriented Design and C++](https://www.youtube.com/watch?v=rX0ItVEVjHc) — Mike Acton 名讲

## 下一章预告
**Ch 03 — AI 命令协议**:本引擎的"心脏"。我们要把 ECS 包装成 AI 可以用文本调用的接口,这是 AIForge 区别于所有其他引擎的关键。
