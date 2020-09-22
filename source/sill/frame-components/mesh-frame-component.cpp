#include <lava/sill/frame-components/mesh-frame-component.hpp>

#include <lava/sill/entity-frame.hpp>
#include <lava/sill/game-engine.hpp>
#include <lava/sill/entity.hpp>
#include <lava/sill/components/mesh-component.hpp>

using namespace lava::sill;

MeshFrameComponent::MeshFrameComponent(EntityFrame& entityFrame, uint8_t sceneIndex)
    : IMesh(entityFrame.engine().scene(sceneIndex), false)
    , m_entityFrame(entityFrame)
{
}

// ----- IFrameComponent

void MeshFrameComponent::makeEntity(Entity& entity)
{
    auto& meshComponent = entity.make<sill::MeshComponent>();
    meshComponent.m_frame = this;
    meshComponent.m_nodes = m_nodes;
    meshComponent.m_animations = m_animations;
    meshComponent.m_nodesDirty = true;

    for (auto& node : meshComponent.m_nodes) {
        if (node.group) {
            for (auto primitive : node.group->primitives()) {
                node.instanceIndex = primitive->addInstance();
            }
        }
    }
}

void MeshFrameComponent::warnEntityRemoved(Entity& entity)
{
    auto& meshComponent = entity.get<sill::MeshComponent>();
    meshComponent.m_frame = nullptr;

    // Remove extra instances
    for (auto& node : m_nodes) {
        if (node.group) {
            for (auto primitive : node.group->primitives()) {
                primitive->removeInstance();
            }
        }
    }

    // Reschedule other instances indices
    std::unordered_map<MeshGroup*, uint32_t> groupsCountMap;
    for (auto entity : m_entityFrame.entities()) {
        auto& meshComponent = entity->get<sill::MeshComponent>();
        meshComponent.m_nodesDirty = true;
        for (auto& node : meshComponent.m_nodes) {
            if (node.group) {
                auto group = node.group.get();
                if (groupsCountMap.find(group) == groupsCountMap.end()) {
                    groupsCountMap[group] = 0u;
                }
                node.instanceIndex = groupsCountMap[group];
                groupsCountMap[group] += 1u;
            }
        }
    }
}
