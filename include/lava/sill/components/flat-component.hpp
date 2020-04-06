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
        void update(float /* dt */) final {}

        /// A flat node holds the geometry hierarchy.
        FlatNode& node(uint32_t index) { return m_nodes[index]; }
        FlatNode& node(const std::string& name);
        std::vector<FlatNode>& nodes() { return m_nodes; }
        const std::vector<FlatNode>& nodes() const { return m_nodes; }
        void nodes(std::vector<FlatNode>&& nodes);

        FlatNode& addNode(); // Emplace back a node.
        void removeNode(const std::string& name);

        // Helper to access a primitive directly.
        magma::Flat& primitive(uint32_t nodeIndex, uint32_t primitiveIndex)
        {
            return m_nodes[nodeIndex].flatGroup->primitive(primitiveIndex);
        }

        // Helper to access a material directly.
        magma::Material* material(uint32_t nodeIndex, uint32_t primitiveIndex)
        {
            return m_nodes[nodeIndex].flatGroup->primitive(primitiveIndex).material();
        }

    protected:
        void onWorldTransform2dChanged();

    private:
        TransformComponent& m_transformComponent;

        std::vector<FlatNode> m_nodes;
    };
}
