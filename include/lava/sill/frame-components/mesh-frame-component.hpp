#pragma once

#include <lava/sill/frame-components/i-frame-component.hpp>
#include <lava/sill/i-mesh.hpp>

#include <lava/sill/mesh-node.hpp>

namespace lava::sill {
    class EntityFrame;
    class Entity;
}

namespace lava::sill {
    class MeshFrameComponent final : public IFrameComponent, public IMesh {
    public:
        MeshFrameComponent(EntityFrame& entityFrame, uint8_t sceneIndex = 0u);

        // IFrameComponent
        static std::string hrid() { return "mesh"; }
        void makeEntity(Entity& entity);
        void warnEntityRemoved(Entity& entity);

    private:
        EntityFrame& m_entityFrame;
    };
}
