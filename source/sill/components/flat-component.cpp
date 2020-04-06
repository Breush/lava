#include <lava/sill/components/flat-component.hpp>

#include <lava/sill/game-entity.hpp>
#include <lava/sill/game-engine.hpp>
#include <lava/sill/components/transform-component.hpp>

using namespace lava;
using namespace lava::sill;

FlatComponent::FlatComponent(GameEntity& entity)
    : IComponent(entity)
    , m_transformComponent(entity.ensure<sill::TransformComponent>())
{
    m_transformComponent.onWorldTransform2dChanged([this]() { onWorldTransform2dChanged(); });
}

// ----- Nodes

FlatNode& FlatComponent::node(const std::string& name)
{
    auto nodeIt = std::find_if(m_nodes.begin(), m_nodes.end(), [&name](const FlatNode& node) {
        return node.name == name;
    });
    return *nodeIt;
}

void FlatComponent::nodes(std::vector<FlatNode>&& nodes)
{
    m_nodes = std::move(nodes);

    // Update all flats transform
    onWorldTransform2dChanged();
}

FlatNode& FlatComponent::addNode()
{
    return m_nodes.emplace_back();
}

void FlatComponent::removeNode(const std::string& name)
{
    auto nodeIt = std::find_if(m_nodes.begin(), m_nodes.end(), [&name](const FlatNode& node) {
        return node.name == name;
    });

    if (nodeIt == m_nodes.end()) return;

    m_nodes.erase(nodeIt);
}

// ----- Internal

void FlatComponent::onWorldTransform2dChanged()
{
    for (const auto& node : m_nodes) {
        if (!node.flatGroup) continue;

        auto transform = m_transformComponent.worldTransform2d() * node.localTransform;

        for (auto primitive : node.flatGroup->primitives()) {
            primitive->transform(transform);
        }
    }
}
