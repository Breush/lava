#pragma once

#include <lava/dike/physics-engine.hpp>
#include <lava/dike/rigid-bodies/i-rigid-body.hpp>

namespace lava::dike {
    class BoxRigidBody final : public IRigidBody {
    public:
        BoxRigidBody(PhysicsEngine& engine, const glm::vec3& dimensions);
        ~BoxRigidBody();

        // Update dimensions after creation.
        void dimensions(const glm::vec3& dimensions);

        // IRigidBody
        bool enabled() const final;
        void enabled(bool enabled) final;

        const glm::mat4& transform() const final;
        void transform(const glm::mat4& transform) final;

        glm::vec3 translation() const final;
        void translate(const glm::vec3& delta) final;

    private:
        class Impl;
        Impl* m_impl = nullptr;
    };
}
