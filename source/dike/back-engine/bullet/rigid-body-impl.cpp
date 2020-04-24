#include "./rigid-body-impl.hpp"

#include "./physics-engine-impl.hpp"

using namespace lava::dike;

RigidBody::Impl::Impl(PhysicsEngine& engine)
    : m_engine(engine.impl())
{
}

RigidBody::Impl::~Impl()
{
    if (m_rigidBody == nullptr) return;

    if (m_enabled) {
        m_engine.dynamicsWorld().removeRigidBody(m_rigidBody.get());
    }
}

// ----- Physics world

void RigidBody::Impl::enabled(bool enabled)
{
    if (m_enabled == enabled) return;
    m_enabled = enabled;
    if (m_rigidBody == nullptr) return;

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
    // @note Bullet physics does not allow for scaling
    // in rigid body transform, thus this decompose is needed,
    // to apply it to the shape instead.

    glm::vec3 scaling, skew;
    glm::vec4 perspective;
    glm::decompose(transform, scaling, m_rotation, m_translation, skew, perspective);

    if (m_scaling != scaling) {
        m_scaling = scaling;
        m_shape.setLocalScaling(btVector3(m_scaling.x, m_scaling.y, m_scaling.z));
        updateShape();
    }

    btVector3 btTranslation(m_translation.x, m_translation.y, m_translation.z);
    btQuaternion btOrientation(m_rotation.x, m_rotation.y, m_rotation.z, m_rotation.w);
    btTransform btWorldTransform;
    btWorldTransform.setOrigin(btTranslation);
    btWorldTransform.setRotation(btOrientation);

    // @note m_transform will be updated within this function
    m_motionState.setWorldTransform(btWorldTransform);

    if (m_rigidBody == nullptr) return;

    m_rigidBody->setWorldTransform(btWorldTransform);

    m_rigidBody->activate(true);
    m_rigidBody->clearForces();
    m_rigidBody->setLinearVelocity(btVector3(0, 0, 0));
    m_rigidBody->setAngularVelocity(btVector3(0, 0, 0));

    // @fixme Still don't know how to activate back everything
    // when we move a non-dynamic object.
}

// ----- Shapes

void RigidBody::Impl::clearShapes()
{
    int32_t shapesCount = m_shape.getNumChildShapes();
    while (shapesCount > 0) {
        shapesCount -= 1;
        m_shape.removeChildShapeByIndex(shapesCount);
    }

    m_shapes.clear();
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
    shape->calculateLocalInertia(m_mass, m_inertia);

    // @note As addChildShape does not apply the current scaling of the compound shape,
    // we first revert it to default before adding the new child and re-applying it.
    m_shape.setLocalScaling(btVector3(1.f, 1.f, 1.f));
    m_shape.addChildShape(localTransform, shape.get());
    m_shape.setLocalScaling(btVector3(m_scaling.x, m_scaling.y, m_scaling.z));
    m_shape.calculateLocalInertia(m_shapes.size() * m_mass, m_inertia);

    updateShape();
}

void RigidBody::Impl::updateShape()
{
    if (m_rigidBody != nullptr) {
        m_engine.dynamicsWorld().removeRigidBody(m_rigidBody.get());
        m_rigidBody = nullptr;
    }

    btRigidBody::btRigidBodyConstructionInfo constructionInfo(m_dynamic ? m_mass : 0.f, &m_motionState, &m_shape, m_inertia);
    constructionInfo.m_restitution = 0.5f;
    m_rigidBody = std::make_unique<btRigidBody>(constructionInfo);

    m_engine.dynamicsWorld().addRigidBody(m_rigidBody.get());
}
