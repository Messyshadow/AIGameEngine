#include "ECS.h"

#include <algorithm>
#include <utility>

namespace AIForge {

Entity::Entity(uint32_t id, std::string name)
    : m_id(id), m_name(std::move(name)) {}

Entity::~Entity() = default;

void Entity::RemoveComponent(const std::string& typeName) {
    m_components.erase(typeName);
}

Component* Entity::GetComponentByType(const std::string& typeName) const {
    auto it = m_components.find(typeName);
    if (it == m_components.end()) return nullptr;
    return it->second.get();
}

std::vector<std::string> Entity::ListComponentTypes() const {
    std::vector<std::string> out;
    out.reserve(m_components.size());
    for (auto& kv : m_components) out.push_back(kv.first);
    std::sort(out.begin(), out.end());
    return out;
}

World::World() = default;
World::~World() = default;

Entity* World::Spawn(const std::string& name) {
    auto entity = std::make_unique<Entity>(m_nextID++, name);
    entity->AddComponent<Transform>();
    Entity* raw = entity.get();
    m_entities.push_back(std::move(entity));
    return raw;
}

Entity* World::Find(const std::string& name) const {
    for (auto& e : m_entities) {
        if (e->GetName() == name) return e.get();
    }
    return nullptr;
}

Entity* World::FindByID(uint32_t id) const {
    for (auto& e : m_entities) {
        if (e->GetID() == id) return e.get();
    }
    return nullptr;
}

std::vector<Entity*> World::FindByComponent(const std::string& typeName) const {
    std::vector<Entity*> out;
    for (auto& e : m_entities) {
        if (e->GetComponentByType(typeName)) out.push_back(e.get());
    }
    return out;
}

void World::Destroy(Entity* entity) {
    if (!entity) return;
    auto it = std::find_if(
        m_entities.begin(), m_entities.end(),
        [entity](const std::unique_ptr<Entity>& e) { return e.get() == entity; });
    if (it != m_entities.end()) m_entities.erase(it);
}

void World::DestroyAll() { m_entities.clear(); }

void World::Update(float /*dt*/) {
    // Phase 1: 占位。后续阶段会调用每个实体上挂载的"可更新组件"的 Update(dt)。
}

}  // namespace AIForge
