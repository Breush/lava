#pragma once

#include <lava/sill/components/i-component.hpp>

#include <lava/sill/mesh-node.hpp>

namespace lava::sill {
    class MeshComponent final : public IComponent {
    public:
        MeshComponent(GameEntity& entity);
        ~MeshComponent();

        // IComponent
        static std::string hrid() { return "mesh"; }
        void update(float dt) override final;

        /// A mesh node holds the geometry hierarchy.
        MeshNode& node(uint32_t index);
        const std::vector<MeshNode>& nodes() const;
        void nodes(std::vector<MeshNode>&& nodes);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
