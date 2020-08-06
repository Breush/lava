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

void RigidBody::Impl::transform(const lava::Transform& transform)
{
    // @note Bullet physics does not allow for scaling
    // in rigid body transform, thus this decompose is needed,
    // to apply it to the shape instead.

    bool scalingChanged = (m_transform.scaling != transform.scaling);
    m_transform = transform;

    if (scalingChanged) {
        m_shape.setLocalScaling(btVector3(transform.scaling, transform.scaling, transform.scaling));
        updateShape();
    }

    btVector3 btTranslation(m_transform.translation.x, m_transform.translation.y, m_transform.translation.z);
    btQuaternion btOrientation(m_transform.rotation.x, m_transform.rotation.y, m_transform.rotation.z, m_transform.rotation.w);
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
    m_meshShapeArrays.clear();
}

void RigidBody::Impl::addBoxShape(const glm::vec3& offset, const glm::vec3& dimensions)
{
    // @fixme For all these shapes, we could get a localTransform instead of offset.

    // @note btBoxShape takes halfExtent
    auto pShape = std::make_unique<btBoxShape>(btVector3{dimensions.x / 2.f, dimensions.y / 2.f, dimensions.z / 2.f});
    addShape(glm::translate(glm::mat4(1.f), offset), std::move(pShape));
}

void RigidBody::Impl::addSphereShape(const glm::vec3& offset, float diameter)
{
    // @note btSphereShape takes radius
    auto pShape = std::make_unique<btSphereShape>(diameter / 2.f);
    addShape(glm::translate(glm::mat4(1.f), offset), std::move(pShape));
}

void RigidBody::Impl::addInfinitePlaneShape(const glm::vec3& offset, const glm::vec3& normal)
{
    auto pShape = std::make_unique<btStaticPlaneShape>(btVector3{normal.x, normal.y, normal.z}, 0.f);
    addShape(glm::translate(glm::mat4(1.f), offset), std::move(pShape));
}

void RigidBody::Impl::addMeshShape(const glm::mat4& localTransform, const VectorView<glm::vec3>& vertices, const std::vector<uint16_t>& indices)
{
    btIndexedMesh indexedMesh;
    indexedMesh.m_numTriangles = indices.size() / 3u;
    indexedMesh.m_triangleIndexBase = reinterpret_cast<const uint8_t*>(&indices[0u]);
    indexedMesh.m_triangleIndexStride = 3u * sizeof(uint16_t);
    indexedMesh.m_numVertices = vertices.size();
    indexedMesh.m_vertexBase = reinterpret_cast<const uint8_t*>(&vertices[0u]);
    indexedMesh.m_vertexStride = vertices.stride();
    indexedMesh.m_indexType = PHY_SHORT;
    indexedMesh.m_vertexType = PHY_FLOAT;

    auto& meshShapeArray = *m_meshShapeArrays.emplace_back(std::make_unique<btTriangleIndexVertexArray>());
    meshShapeArray.addIndexedMesh(indexedMesh, PHY_SHORT);

    auto pShape = std::make_unique<btBvhTriangleMeshShape>(&meshShapeArray, true);
    addShape(localTransform, std::move(pShape));
}

// ----- Helpers

float RigidBody::Impl::distanceFrom(const Ray& ray, float maxDistance) const
{
    btVector3 from(ray.origin.x, ray.origin.y, ray.origin.z);
    btVector3 direction(ray.direction.x, ray.direction.y, ray.direction.z);
    btVector3 to = from + maxDistance * direction;

    btCollisionWorld::ClosestRayResultCallback closestResults(from, to);
    closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

    btTransform fromTransform(btQuaternion(0.f, 0.f, 0.f, 1.f), from);
    btTransform toTransform(btQuaternion(0.f, 0.f, 0.f, 1.f), to);
    btTransform transform = m_rigidBody->getWorldTransform();

    m_engine.dynamicsWorld().rayTestSingle(fromTransform, toTransform, m_rigidBody.get(),
                                           &m_shape, transform, closestResults);

    if (closestResults.hasHit()) {
        return (closestResults.m_hitPointWorld - from).length();
    }

    return 0.f;
}

// ----- Internal

void RigidBody::Impl::addShape(const glm::mat4& localTransform, std::unique_ptr<btCollisionShape>&& pShape)
{
    glm::vec3 translation, scaling, skew;
    glm::vec4 perspective;
    glm::quat rotation;
    glm::decompose(localTransform, scaling, rotation, translation, skew, perspective);

    btTransform btLocalTransform;
    btLocalTransform.setOrigin(btVector3{translation.x, translation.y, translation.z});
    btLocalTransform.setRotation(btQuaternion{rotation.x, rotation.y, rotation.z, rotation.w});

    auto& shape = m_shapes.emplace_back();
    shape = std::move(pShape);
    shape->calculateLocalInertia(m_mass, m_inertia);
    shape->setLocalScaling(btVector3{scaling.x, scaling.y, scaling.z});

    // @note As addChildShape does not apply the current global scaling of the compound shape,
    // we first revert it to default before adding the new child and re-applying it.
    m_shape.setLocalScaling(btVector3(1.f, 1.f, 1.f));
    m_shape.addChildShape(btLocalTransform, shape.get());
    m_shape.setLocalScaling(btVector3{m_transform.scaling, m_transform.scaling, m_transform.scaling});
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
