#pragma once

#include <lava/dike/physics-engine.hpp>
#include <lava/dike/rigid-bodies/i-rigid-body.hpp>

namespace lava::dike {
    class SphereRigidBody : public IRigidBody {
    public:
        SphereRigidBody(PhysicsEngine& engine, float radius);
        ~SphereRigidBody();

        // IRigidBody
        glm::vec3 position() const override;
        void positionAdd(const glm::vec3& delta) override;

    private:
        class Impl;
        Impl* m_impl = nullptr;
    };
}
