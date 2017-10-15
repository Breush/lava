#pragma once

#include <lava/sill/components/transform-component.hpp>

#include "./component-impl.hpp"

namespace lava::sill {
    class TransformComponent::Impl : public ComponentImpl {
    public:
        Impl(GameEntity& entity);

        // IComponent
        void update() override final {}
        void postUpdate() override final;

        // TransformComponent
        bool changed() const { return m_changed; }
        const glm::mat4& worldTransform() const { return m_transform; } // @todo Concept of nodes/worldTransform
        void positionAdd(const glm::vec3& delta);

    private:
        // Data
        glm::mat4 m_transform;
        bool m_changed = true;
    };
}
