#pragma once

#include <lava/dike/static-rigid-bodies/plane-static-rigid-body.hpp>

#include <bullet/btBulletDynamicsCommon.h>
#include <glm/vec3.hpp>

namespace lava::dike {
    class PlaneStaticRigidBody::Impl {
    public:
        Impl(PhysicsEngine& engine, const glm::vec3& normal);
        ~Impl();

    private:
        btStaticPlaneShape m_shape;
        btRigidBody::btRigidBodyConstructionInfo m_constructionInfo;
        std::unique_ptr<btRigidBody> m_rigidBody;
    };
}
