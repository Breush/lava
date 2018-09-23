#pragma once

#include <lava/sill/components/camera-component.hpp>

#include "./component-impl.hpp"

namespace lava::sill {
    class CameraComponent::Impl : public ComponentImpl {
    public:
        Impl(GameEntity& entity);
        ~Impl();

        // IComponent
        void update(float dt) override final;

        // CameraComponent
        const glm::vec3& translation() const { return m_camera->translation(); }
        void translation(const glm::vec3& translation) { m_camera->translation(translation); }

        const glm::vec3& target() const { return m_camera->target(); }
        void target(const glm::vec3& target) { m_camera->target(target); }

        float radius() const { return m_camera->radius(); }
        void radius(float radius) { m_camera->radius(radius); }

        void strafe(float x, float y) { m_camera->strafe(x, y); }
        void radiusAdd(float radiusDistance) { m_camera->radiusAdd(radiusDistance); }
        void orbitAdd(float longitudeAngle, float latitudeAngle) { m_camera->orbitAdd(longitudeAngle, latitudeAngle); }

    private:
        magma::OrbitCamera* m_camera = nullptr;

        Extent2d m_extent;
        float m_updateDelay = -1.f;
    };
}
