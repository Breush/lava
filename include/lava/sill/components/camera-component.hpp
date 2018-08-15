#pragma once

#include <lava/sill/components/i-component.hpp>

#include <glm/vec3.hpp>
#include <string>

namespace lava::sill {
    class CameraComponent final : public IComponent {
    public:
        CameraComponent(GameEntity& entity);
        ~CameraComponent();

        // IComponent
        static std::string hrid() { return "camera"; }
        void update() override final;

        // Public API
        const glm::vec3& translation() const;
        void translation(const glm::vec3& translation);

        const glm::vec3& target() const;
        void target(const glm::vec3& target);

        float radius() const;
        void radius(float radius);

        void strafe(float x, float y);
        void radiusAdd(float radiusDistance);
        void orbitAdd(float longitudeAngle, float latitudeAngle);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
