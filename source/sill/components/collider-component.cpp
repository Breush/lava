#include <lava/sill/components/collider-component.hpp>

#include <lava/sill/components/mesh-component.hpp>
#include <lava/sill/components/transform-component.hpp>
#include <lava/sill/game-engine.hpp>
#include <lava/sill/game-entity.hpp>
#include <lava/sill/makers/box-mesh.hpp>

#include "./physics-component-impl.hpp"

using namespace lava;
using namespace lava::sill;

namespace {
    void addMeshNodeShape(dike::RigidBody& rigidBody, const MeshNode& node, const glm::mat4& parentTransform)
    {
        auto transform = parentTransform * node.localTransform;

        if (node.meshGroup) {
            for (auto& primitive : node.meshGroup->primitives()) {
                auto& unlitVertices = primitive->unlitVertices();
                VectorView<glm::vec3> vertices(reinterpret_cast<uint8_t*>(unlitVertices.data()) + offsetof(magma::UnlitVertex, pos), unlitVertices.size(), sizeof(magma::UnlitVertex));
                rigidBody.addMeshShape(transform, vertices, primitive->indices());
            }
        }

        for (auto child : node.children) {
            addMeshNodeShape(rigidBody, *child, transform);
        }
    }
}

ColliderComponent::ColliderComponent(GameEntity& entity)
    : IComponent(entity)
    , m_transformComponent(entity.ensure<TransformComponent>())
    , m_physicsComponent(entity.ensure<PhysicsComponent>())
{
    m_transformComponent.onWorldTransformChanged([this]() { onWorldTransformChanged(); });
}

ColliderComponent::~ColliderComponent()
{
    // Clean-up
    debugEnabled(false);

    m_boxShapes.clear();
    m_sphereShapes.clear();
    m_infinitePlaneShapes.clear();
}

//----- Shapes

void ColliderComponent::clearShapes()
{
    m_boxShapes.clear();
    m_sphereShapes.clear();
    m_infinitePlaneShapes.clear();
    m_physicsComponent.impl().dike().clearShapes();

    // Refresh debug
    if (m_debugEnabled) {
        debugEnabled(false);
        debugEnabled(true);
    }
}

void ColliderComponent::addBoxShape(const glm::vec3& offset, const glm::vec3& dimensions)
{
    m_boxShapes.emplace_back(BoxShape{offset, dimensions});
    m_physicsComponent.impl().dike().addBoxShape(offset, dimensions);

    // Refresh debug
    if (m_debugEnabled) {
        debugEnabled(false);
        debugEnabled(true);
    }
}

void ColliderComponent::addSphereShape(const glm::vec3& offset, float diameter)
{
    m_sphereShapes.emplace_back(SphereShape{offset, diameter});
    m_physicsComponent.impl().dike().addSphereShape(offset, diameter);

    // Refresh debug
    if (m_debugEnabled) {
        debugEnabled(false);
        debugEnabled(true);
    }
}

void ColliderComponent::addInfinitePlaneShape(const glm::vec3& offset, const glm::vec3& normal)
{
    m_infinitePlaneShapes.emplace_back(InfinitePlaneShape{offset, normal});
    m_physicsComponent.impl().dike().addInfinitePlaneShape(offset, normal);

    // Refresh debug
    if (m_debugEnabled) {
        debugEnabled(false);
        debugEnabled(true);
    }
}

void ColliderComponent::addMeshShape()
{
    auto& meshComponent = m_entity.get<MeshComponent>();
    auto& rigidBody = m_physicsComponent.impl().dike();

    // @note The root nodes have just no parent!
    for (auto& node : meshComponent.nodes()) {
        if (node.parent != nullptr) continue;
        addMeshNodeShape(rigidBody, node, glm::mat4(1.f));
    }
}

//----- Debug

void ColliderComponent::debugEnabled(bool debugEnabled)
{
    if (m_debugEnabled == debugEnabled) return;
    m_debugEnabled = debugEnabled;

    if (debugEnabled) {
        for (auto& boxShape : m_boxShapes) {
            boxShape.debugEntity = &m_entity.engine().make<GameEntity>();
            auto& debugMeshComponent = boxShape.debugEntity->make<MeshComponent>();

            makers::BoxMeshOptions options;
            options.offset = boxShape.offset;
            makers::boxMeshMaker(boxShape.extent, options)(debugMeshComponent);
            debugMeshComponent.category(RenderCategory::Wireframe);
        }

        // @todo Debug for spheres and infinite planes
    }
    else {
        for (auto& boxShape : m_boxShapes) {
            m_entity.engine().remove(*boxShape.debugEntity);
            boxShape.debugEntity = nullptr;
        }
    }
}

//----- Callbacks

void ColliderComponent::onWorldTransformChanged()
{
    if (m_debugEnabled) {
        const auto& worldTransform = m_transformComponent.worldTransform();
        for (auto& boxShape : m_boxShapes) {
            boxShape.debugEntity->get<TransformComponent>().worldTransform(worldTransform);
        }
    }
}
