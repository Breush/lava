#pragma once

#include <lava/sill/components/collider-component.hpp>
#include <lava/sill/components/transform-component.hpp>

#include "./component-impl.hpp"

namespace lava::sill {
    class PhysicsComponent;
}

namespace lava::sill {
    class ColliderComponent::Impl : public ComponentImpl {
    public:
        Impl(GameEntity& entity);
        ~Impl();

        // ColliderComponent
        void clearShapes();
        void addBoxShape(const glm::vec3& offset, const glm::vec3& dimensions);
        void addSphereShape(const glm::vec3& offset, float diameter);
        void addInfinitePlaneShape(const glm::vec3& offset, const glm::vec3& normal);

        void debugEnabled(bool debugEnabled);

    protected:
        /// Callbacks
        void onWorldTransformChanged();

    protected:
        struct BoxShape {
            glm::vec3 offset;
            glm::vec3 dimensions;
            GameEntity* debugEntity = nullptr;
        };

        struct SphereShape {
            glm::vec3 offset;
            float diameter;
            GameEntity* debugEntity = nullptr;
        };

        struct InfinitePlaneShape {
            glm::vec3 offset;
            glm::vec3 normal;
            GameEntity* debugEntity = nullptr;
        };

    private:
        // References
        TransformComponent& m_transformComponent;
        PhysicsComponent& m_physicsComponent;

        // Resources
        std::vector<BoxShape> m_boxShapes;
        std::vector<SphereShape> m_sphereShapes;
        std::vector<InfinitePlaneShape> m_infinitePlaneShapes;

        // Debug
        bool m_debugEnabled = false;
    };
}
