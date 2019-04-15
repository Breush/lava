#include "./mesh-primitive-impl.hpp"

#include "./game-engine-impl.hpp"

#include <lava/sill/material.hpp>

using namespace lava::sill;

MeshPrimitive::Impl::Impl(GameEngine& engine)
    : m_engine(engine.impl())
{
    m_magma = &m_engine.renderScene().make<magma::Mesh>();
}

MeshPrimitive::Impl::~Impl()
{
    m_engine.renderScene().remove(*m_magma);
    m_magma = nullptr;
}

//----- Mesh

void MeshPrimitive::Impl::verticesCount(const uint32_t count)
{
    m_magma->verticesCount(count);
}

void MeshPrimitive::Impl::verticesPositions(VectorView<glm::vec3> positions)
{
    m_magma->verticesPositions(positions);
}

void MeshPrimitive::Impl::verticesUvs(VectorView<glm::vec2> uvs)
{
    m_magma->verticesUvs(uvs);
}

void MeshPrimitive::Impl::verticesNormals(VectorView<glm::vec3> normals)
{
    m_magma->verticesNormals(normals);
}

void MeshPrimitive::Impl::verticesTangents(VectorView<glm::vec4> tangents)
{
    m_magma->verticesTangents(tangents);
}

void MeshPrimitive::Impl::indices(VectorView<uint16_t> indices)
{
    m_magma->indices(indices);
}

void MeshPrimitive::Impl::material(Material& material)
{
    m_magma->material(material.magma());
}

void MeshPrimitive::Impl::translucent(bool translucent)
{
    m_magma->translucent(translucent);
}
