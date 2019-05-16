#include "./rigid-body-impl.hpp"

#include "./physics-engine-impl.hpp"

using namespace lava::dike;

RigidBody::Impl::Impl(PhysicsEngine& engine)
    : m_engine(engine.impl())
{
}

RigidBody::Impl::~Impl()
{
    if (m_enabled) {
        m_engine.dynamicsWorld().removeRigidBody(m_rigidBody.get());
    }
}

// ----- Physics world

void RigidBody::Impl::enabled(bool enabled)
{
    if (m_enabled == enabled) return;
    m_enabled = enabled;

    if (!enabled) {
        m_engine.dynamicsWorld().removeRigidBody(m_rigidBody.get());
    }
    else {
        m_engine.dynamicsWorld().addRigidBody(m_rigidBody.get());
    }
}

void RigidBody::Impl::dynamic(bool dynamic)
{
    if (m_dynamic == dynamic) return;
    m_dynamic = dynamic;

    updateShape();
}

void RigidBody::Impl::transform(const glm::mat4& transform)
{
    m_transform = transform;
    if (m_rigidBody == nullptr) return;

    btTransform worldTransform;
    worldTransform.setFromOpenGLMatrix(reinterpret_cast<float*>(&m_transform));

    m_rigidBody->setWorldTransform(worldTransform);
    m_motionState.setWorldTransform(worldTransform);

    m_rigidBody->activate(true);
    m_rigidBody->clearForces();
    m_rigidBody->setLinearVelocity(btVector3(0, 0, 0));
    m_rigidBody->setAngularVelocity(btVector3(0, 0, 0));
}

// ----- Shapes

void RigidBody::Impl::clearShapes()
{
    int32_t shapesCount = m_shape.getNumChildShapes();
    while (shapesCount > 0) {
        shapesCount -= 1;
        m_shape.removeChildShapeByIndex(shapesCount);
    }
}

void RigidBody::Impl::addBoxShape(const glm::vec3& offset, const glm::vec3& dimensions)
{
    // @note btBoxShape takes halfExtent
    auto pShape = std::make_unique<btBoxShape>(btVector3{dimensions.x / 2.f, dimensions.y / 2.f, dimensions.z / 2.f});
    addShape(offset, std::move(pShape));
}

void RigidBody::Impl::addSphereShape(const glm::vec3& offset, float diameter)
{
    // @note btSphereShape takes radius
    auto pShape = std::make_unique<btSphereShape>(diameter / 2.f);
    addShape(offset, std::move(pShape));
}

void RigidBody::Impl::addInfinitePlaneShape(const glm::vec3& offset, const glm::vec3& normal)
{
    auto pShape = std::make_unique<btStaticPlaneShape>(btVector3{normal.x, normal.y, normal.z}, 0.f);
    addShape(offset, std::move(pShape));
}

// ----- Internal

void RigidBody::Impl::addShape(const glm::vec3& offset, std::unique_ptr<btCollisionShape>&& pShape)
{
    btTransform localTransform;
    localTransform.setIdentity();
    localTransform.setOrigin(btVector3{offset.x, offset.y, offset.z});

    auto& shape = m_shapes.emplace_back();
    shape = std::move(pShape);

    if (m_dynamic) {
        shape->calculateLocalInertia(m_mass, m_inertia);
    }

    m_shape.addChildShape(localTransform, shape.get());
    updateShape();
}

void RigidBody::Impl::updateShape()
{
    if (m_rigidBody != nullptr) {
        m_engine.dynamicsWorld().removeRigidBody(m_rigidBody.get());
    }

    btRigidBody::btRigidBodyConstructionInfo constructionInfo(m_dynamic ? m_mass : 0.f, &m_motionState, &m_shape, m_inertia);
    constructionInfo.m_restitution = 0.5f;
    m_rigidBody = std::make_unique<btRigidBody>(constructionInfo);

    m_engine.dynamicsWorld().addRigidBody(m_rigidBody.get());
}
