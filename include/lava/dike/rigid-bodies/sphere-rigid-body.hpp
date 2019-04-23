#pragma once

#include <lava/dike/physics-engine.hpp>
#include <lava/dike/rigid-bodies/i-rigid-body.hpp>

namespace lava::dike {
    class SphereRigidBody final : public IRigidBody {
    public:
        SphereRigidBody(PhysicsEngine& engine, float diameter);
        ~SphereRigidBody();

        // IRigidBody
        const glm::mat4& transform() const final;
        void transform(const glm::mat4& transform) final;

        glm::vec3 translation() const final;
        void translate(const glm::vec3& delta) final;

    private:
        class Impl;
        Impl* m_impl = nullptr;
    };
}
