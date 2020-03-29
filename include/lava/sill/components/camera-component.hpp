#pragma once

#include <lava/sill/components/i-component.hpp>

#include <glm/vec3.hpp>
#include <lava/core/extent.hpp>
#include <lava/core/ray.hpp>
#include <lava/magma/frustum.hpp>
#include <string>

// @fixme Wouldn't be shocking if we had access to the underlying magma::Camera,
// as we do use magma::Material and such directly.

namespace lava::sill {
    class CameraComponent final : public IComponent {
    public:
        CameraComponent(GameEntity& entity);
        ~CameraComponent();

        // IComponent
        static std::string hrid() { return "camera"; }
        void update(float dt) override final;

        // Public API
        const Extent2d& extent() const;

        const glm::vec3& origin() const;
        void origin(const glm::vec3& origin);

        const glm::vec3& target() const;
        void target(const glm::vec3& target);

        float radius() const;
        void radius(float radius);

        void strafe(float x, float y);
        void radiusAdd(float radiusDistance);
        void orbitAdd(float longitudeAngle, float latitudeAngle);
        void rotateAtOrigin(float longitudeAngle, float latitudeAngle);

        void goForward(float distance, const glm::vec3& constraints = {1, 1, 1});
        void goRight(float distance, const glm::vec3& constraints = {1, 1, 1});

        const glm::mat4& viewTransform() const;
        const glm::mat4& projectionTransform() const;

        /// Transform the render target position into a ray going forward.
        Ray coordinatesToRay(const glm::vec2& coordinates) const;

        /// Transform screen-space coordinates to a 3D position.
        glm::vec3 unproject(const glm::vec2& coordinates, float depth = 0.f) const;

        /// Transform screen-space rectangle to a 3D view frustum.
        magma::Frustum frustum(const glm::vec2& topLeftCoordinates, const glm::vec2& bottomRightCoordinates) const;

        /// Get the transform to front-looking show a 3D element at coordinates.
        glm::mat4 transformAtCoordinates(const glm::vec2& coordinates, float depth = 0.f) const;

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
