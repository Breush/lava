#include "./mesh-component-impl.hpp"

#include <lava/sill/components/transform-component.hpp>

#include "../mesh-impl.hpp"

using namespace lava;

namespace {
    void updateNodeTransforms(sill::MeshNode& node, const glm::mat4& parentTransform)
    {
        auto transform = parentTransform * node.transform;

        if (node.mesh) {
            node.mesh->impl().transform(transform);
        }

        for (auto child : node.children) {
            updateNodeTransforms(*child, transform);
        }
    }
}

using namespace lava::sill;

MeshComponent::Impl::Impl(GameEntity& entity)
    : ComponentImpl(entity)
    , m_transformComponent(entity.ensure<TransformComponent>())
{
    m_transformComponent.onTransformChanged([this]() { onTransformChanged(); });

    // Init correctly on first creation
    onTransformChanged();
}

// ----- MeshComponent

void MeshComponent::Impl::nodes(std::vector<MeshNode>&& nodes)
{
    m_nodes = std::move(nodes);

    // Affect parents to each node.
    for (auto& node : m_nodes) {
        node.parent = nullptr;
    }

    for (auto& node : m_nodes) {
        for (auto& child : node.children) {
            child->parent = &node;
        }
    }

    // Update all meshes transform
    onTransformChanged();
}

// ----- Internal

void MeshComponent::Impl::onTransformChanged()
{
    // @todo We can be more clever than this and dirtify the transforms,
    // waiting for the next update cycle to effectively update.

    auto modelTransform = m_transformComponent.worldTransform();

    // @note The root nodes have just no parent!
    for (auto& node : m_nodes) {
        if (node.parent != nullptr) continue;
        updateNodeTransforms(node, modelTransform);
    }
}
