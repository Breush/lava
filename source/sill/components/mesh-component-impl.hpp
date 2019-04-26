#pragma once

#include <lava/sill/components/mesh-component.hpp>

#include "./component-impl.hpp"

namespace lava::sill {
    class TransformComponent;
    class Material;
}

namespace lava::sill {
    class MeshComponent::Impl : public ComponentImpl {
    public:
        Impl(GameEntity& entity);

        // IComponent
        void update(float /* dt */) override final {}

        // MeshComponent
        MeshNode& node(uint32_t index) { return m_nodes[index]; }
        std::vector<MeshNode>& nodes() { return m_nodes; }
        const std::vector<MeshNode>& nodes() const { return m_nodes; }
        void nodes(std::vector<MeshNode>&& nodes);

        // Callbacks
        void onWorldTransformChanged();

    private:
        // References
        TransformComponent& m_transformComponent;

        // Resources
        std::vector<MeshNode> m_nodes;
    };
}
