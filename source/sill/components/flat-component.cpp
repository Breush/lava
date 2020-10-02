#include <lava/sill/components/flat-component.hpp>

#include <lava/sill/entity.hpp>
#include <lava/sill/game-engine.hpp>
#include <lava/sill/components/transform-component.hpp>

using namespace lava;
using namespace lava::sill;

FlatComponent::FlatComponent(Entity& entity)
    : IComponent(entity)
    , m_transformComponent(entity.ensure<sill::TransformComponent>())
{
    m_transformComponent.onWorldTransform2dChanged([this]() {
        for (auto& node : m_nodes) {
            node.localTransformChanged = true;
        }
    });
}

void FlatComponent::updateFrame()
{
    // Updating node transforms
    // @todo No node parenting here?
    for (auto& node : m_nodes) {
        if (!node.flatGroup) continue;

        if (node.localTransformChanged) {
            auto transform = m_transformComponent.worldTransform2d().matrix() * node.localTransform;
            node.flatGroup->transform(transform);
            node.localTransformChanged = false;
        }
    }
}

// ----- Nodes

FlatNode& FlatComponent::node(const std::string& name)
{
    auto nodeIt = std::find_if(m_nodes.begin(), m_nodes.end(), [&name](const FlatNode& node) {
        return node.name == name;
    });
    return *nodeIt;
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

    for (auto& node : m_nodes) {
        node.localTransformChanged = true;
    }
}

void FlatComponent::removeNodes(const std::string& name)
{
    auto nodeIt = std::remove_if(m_nodes.begin(), m_nodes.end(), [&name](const FlatNode& node) {
        return node.name == name;
    });

    if (nodeIt == m_nodes.end()) return;

    m_nodes.erase(nodeIt, m_nodes.end());

    for (auto& node : m_nodes) {
        node.localTransformChanged = true;
    }
}

// ----- Helpers

magma::Flat& FlatComponent::primitive(uint32_t nodeIndex, uint32_t primitiveIndex)
{
    return m_nodes[nodeIndex].flatGroup->primitive(primitiveIndex);
}

magma::MaterialPtr FlatComponent::material(uint32_t nodeIndex, uint32_t primitiveIndex)
{
    return m_nodes[nodeIndex].flatGroup->primitive(primitiveIndex).material();
}
