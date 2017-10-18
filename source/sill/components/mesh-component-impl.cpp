#include "./mesh-component-impl.hpp"

#include <lava/sill/components/transform-component.hpp>

#include "../game-engine-impl.hpp"
#include "../game-entity-impl.hpp"
#include "../material-impl.hpp"

using namespace lava::sill;

MeshComponent::Impl::Impl(GameEntity& entity)
    : ComponentImpl(entity)
    , m_transformComponent(entity.ensure<TransformComponent>())
{
    m_mesh = &m_entity.engine().renderScene().make<magma::Mesh>();
}

MeshComponent::Impl::~Impl()
{
    m_entity.engine().renderScene().remove(*m_mesh);
    m_mesh = nullptr;
}

//----- IComponent

void MeshComponent::Impl::update()
{
    // If the transform of the entity moved, update the mesh world position
    if (m_transformComponent.changed()) {
        m_mesh->transform(m_transformComponent.worldTransform());
    }
}

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

void MeshComponent::Impl::material(Material& material)
{
    m_mesh->material(material.impl().material());
}
