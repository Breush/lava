#pragma once

#include <lava/dike/physics-engine.hpp>
#include <lava/dike/static-rigid-bodies/i-static-rigid-body.hpp>

#include <glm/vec3.hpp>

namespace lava::dike {
    class PlaneStaticRigidBody : public IStaticRigidBody {
    public:
        PlaneStaticRigidBody(PhysicsEngine& engine, const glm::vec3& normal);
        ~PlaneStaticRigidBody();

    private:
        class Impl;
        Impl* m_impl = nullptr;
    };
}
