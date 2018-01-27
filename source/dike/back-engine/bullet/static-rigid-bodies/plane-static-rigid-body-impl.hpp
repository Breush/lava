#pragma once

#include <lava/dike/static-rigid-bodies/plane-static-rigid-body.hpp>

#include <glm/vec3.hpp>
#include <bullet/btBulletDynamicsCommon.h>

namespace lava::dike {
    class PlaneStaticRigidBody::Impl {
    public:
        Impl(PhysicsEngine& engine, const glm::vec3& normal);

    private:
        btStaticPlaneShape m_shape;
        btDefaultMotionState m_motionState;
        btRigidBody m_rigidBody;
    };
}
