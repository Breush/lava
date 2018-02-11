#include "./mesh-component-impl.hpp"

#include <lava/sill/components/transform-component.hpp>

#include "../game-engine-impl.hpp"
#include "../game-entity-impl.hpp"

using namespace lava::sill;

MeshComponent::Impl::Impl(GameEntity& entity)
    : ComponentImpl(entity)
    , m_transformComponent(entity.ensure<TransformComponent>())
{
    m_mesh = &m_entity.engine().renderScene().make<magma::Mesh>();

    m_transformComponent.onPositionChanged([this](const glm::vec3&) { onTransformChanged(); });

    // Init correctly on first creation
    onTransformChanged();
}

MeshComponent::Impl::~Impl()
{
    m_entity.engine().renderScene().remove(*m_mesh);
    m_mesh = nullptr;
}

//----- IComponent

void MeshComponent::Impl::verticesCount(const uint32_t count)
{
    m_mesh->verticesCount(count);
}

void MeshComponent::Impl::verticesPositions(const std::vector<glm::vec3>& positions)
{
    m_mesh->verticesPositions(positions);
}

void MeshComponent::Impl::verticesUvs(const std::vector<glm::vec2>& uvs)
{
    m_mesh->verticesUvs(uvs);
}

void MeshComponent::Impl::verticesNormals(const std::vector<glm::vec3>& normals)
{
    m_mesh->verticesNormals(normals);
}

void MeshComponent::Impl::verticesTangents(const std::vector<glm::vec4>& tangents)
{
    m_mesh->verticesTangents(tangents);
}

void MeshComponent::Impl::indices(const std::vector<uint16_t>& indices)
{
    m_mesh->indices(indices);
}

bool MeshComponent::Impl::translucent() const
{
    return m_mesh->translucent();
}

void MeshComponent::Impl::translucent(bool translucent)
{
    m_mesh->translucent(translucent);
}

void MeshComponent::Impl::material(Material& material)
{
    m_mesh->material(material.original());
}

// Internal

void MeshComponent::Impl::onTransformChanged()
{
    m_mesh->transform(m_transformComponent.worldTransform());
}
