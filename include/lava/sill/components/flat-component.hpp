#pragma once

#include <lava/sill/components/i-component.hpp>

#include <lava/sill/flat-node.hpp>

namespace lava::sill {
    class TransformComponent;
}

namespace lava::sill {
    class FlatComponent : public IComponent {
    public:
        FlatComponent(GameEntity& entity);

        // IComponent
        static std::string hrid() { return "flat"; }
        void updateFrame() final;

        /// A flat node holds the geometry hierarchy.
        FlatNode& node(uint32_t index) { return m_nodes[index]; }
        FlatNode& node(const std::string& name);
        std::vector<FlatNode>& nodes() { return m_nodes; }
        const std::vector<FlatNode>& nodes() const { return m_nodes; }

        FlatNode& addNode(); // Emplace back a node.
        void removeNode(const std::string& name);

        // Helpers
        magma::Flat& primitive(uint32_t nodeIndex, uint32_t primitiveIndex);
        magma::MaterialPtr material(uint32_t nodeIndex, uint32_t primitiveIndex);

    private:
        TransformComponent& m_transformComponent;

        std::vector<FlatNode> m_nodes;
    };
}
