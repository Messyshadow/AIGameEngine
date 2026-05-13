#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Math.h"

namespace AIForge {

/// @ai_summary 所有组件的基类。子类必须实现 GetType() 并提供静态 TypeName()。
/// @ai_example
///   struct Health : Component {
///       int hp = 100;
///       static constexpr const char* TypeName() { return "Health"; }
///       const char* GetType() const override { return TypeName(); }
///   };
/// @ai_related Entity, World, Transform
struct Component {
    virtual ~Component() = default;

    /// @ai_summary 返回组件类型名（运行时用）。子类必须重写。
    virtual const char* GetType() const = 0;
};

/// @ai_summary 位置/旋转/缩放组件。每个实体默认会被赋予一个 Transform。
/// @ai_params position 世界坐标
/// @ai_params rotation 欧拉角（度）
/// @ai_params scale 缩放
/// @ai_example
///   auto* t = entity->GetComponent<Transform>();
///   t->position = {10, 0, 5};
/// @ai_related Component, Entity
struct Transform : Component {
    Vec3 position{0, 0, 0};
    Vec3 rotation{0, 0, 0};
    Vec3 scale{1, 1, 1};

    static constexpr const char* TypeName() { return "Transform"; }
    const char* GetType() const override { return TypeName(); }
};

/// @ai_summary 实体（Entity）：可被命名的容器，挂载若干组件。
/// @ai_summary 不要直接构造 Entity；用 World::Spawn 创建。
/// @ai_example
///   auto* e = world.Spawn("player");
///   e->AddComponent<Health>(100);
///   if (auto* h = e->GetComponent<Health>()) h->hp -= 10;
/// @ai_related World, Component, Transform
class Entity {
public:
    Entity(uint32_t id, std::string name);
    ~Entity();

    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    uint32_t GetID() const { return m_id; }
    const std::string& GetName() const { return m_name; }
    void SetName(const std::string& name) { m_name = name; }

    bool IsActive() const { return m_active; }
    void SetActive(bool active) { m_active = active; }

    /// @ai_summary 添加组件，若同类型已存在则替换。
    /// @ai_params Args 任何转发给组件构造函数的参数
    /// @ai_example entity->AddComponent<Health>(100);
    /// @ai_related GetComponent, HasComponent, RemoveComponent
    template <typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        static_assert(std::is_base_of<Component, T>::value,
                      "T must derive from Component");
        auto comp = std::make_unique<T>(std::forward<Args>(args)...);
        T* raw = comp.get();
        m_components[T::TypeName()] = std::move(comp);
        return raw;
    }

    /// @ai_summary 获取组件指针，没有则返回 nullptr。
    /// @ai_example if (auto* t = e->GetComponent<Transform>()) { ... }
    /// @ai_related AddComponent, HasComponent
    template <typename T>
    T* GetComponent() {
        auto it = m_components.find(T::TypeName());
        if (it == m_components.end()) return nullptr;
        return static_cast<T*>(it->second.get());
    }

    /// @ai_summary const 重载,供只读上下文(如 AIContext)使用
    template <typename T>
    const T* GetComponent() const {
        auto it = m_components.find(T::TypeName());
        if (it == m_components.end()) return nullptr;
        return static_cast<const T*>(it->second.get());
    }

    /// @ai_summary 是否拥有指定组件
    template <typename T>
    bool HasComponent() const {
        return m_components.find(T::TypeName()) != m_components.end();
    }

    /// @ai_summary 按类型名移除组件
    void RemoveComponent(const std::string& typeName);

    /// @ai_summary 按类型名获取组件（运行时字符串查询，AI/命令系统用）
    Component* GetComponentByType(const std::string& typeName) const;

    /// @ai_summary 列出当前所有组件类型名
    std::vector<std::string> ListComponentTypes() const;

    const std::unordered_map<std::string, std::unique_ptr<Component>>&
    GetComponents() const {
        return m_components;
    }

private:
    uint32_t m_id;
    std::string m_name;
    bool m_active = true;
    std::unordered_map<std::string, std::unique_ptr<Component>> m_components;
};

/// @ai_summary 世界（World）：所有实体的容器。每个 App 持有一个 World。
/// @ai_summary 实体的创建/销毁/查询都通过 World 完成。
/// @ai_example
///   auto* e = world.Spawn("zombie");
///   auto* t = e->GetComponent<Transform>();
///   t->position = {10, 0, 5};
/// @ai_related Entity, App
class World {
public:
    World();
    ~World();

    World(const World&) = delete;
    World& operator=(const World&) = delete;

    /// @ai_summary 生成新实体（自动获得 Transform 组件）。重名仍允许，但 Find 只返回第一个。
    /// @ai_params name 实体名（不必唯一，但建议唯一）
    /// @ai_example auto* e = world.Spawn("player");
    /// @ai_related Find, Destroy
    Entity* Spawn(const std::string& name);

    /// @ai_summary 按名字查找首个匹配实体，找不到返回 nullptr
    Entity* Find(const std::string& name) const;

    /// @ai_summary 按 ID 查找实体
    Entity* FindByID(uint32_t id) const;

    /// @ai_summary 列出拥有指定组件类型的所有实体
    /// @ai_example auto enemies = world.FindByComponent("Health");
    std::vector<Entity*> FindByComponent(const std::string& typeName) const;

    /// @ai_summary 销毁实体（立即移除）。重复调用安全。
    void Destroy(Entity* entity);

    /// @ai_summary 清空所有实体
    void DestroyAll();

    /// @ai_summary 推进一帧；Phase 1 仅作占位，未来用于驱动组件 Update。
    void Update(float dt);

    int GetEntityCount() const { return static_cast<int>(m_entities.size()); }

    const std::vector<std::unique_ptr<Entity>>& GetEntities() const {
        return m_entities;
    }

private:
    std::vector<std::unique_ptr<Entity>> m_entities;
    uint32_t m_nextID = 1;
};

}  // namespace AIForge
