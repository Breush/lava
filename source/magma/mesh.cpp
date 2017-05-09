#include <lava/magma/mesh.hpp>

#include "./vulkan/mesh-impl.hpp"

using namespace lava;

Mesh::Mesh(Engine& engine)
{
    m_impl = new Impl(engine);
}

Mesh::~Mesh()
{
    delete m_impl;
}

void Mesh::verticesCount(const uint32_t count)
{
    m_impl->verticesCount(count);
}

void Mesh::verticesPositions(const std::vector<glm::vec3>& positions)
{
    m_impl->verticesPositions(positions);
}

void Mesh::verticesColors(const std::vector<glm::vec3>& colors)
{
    m_impl->verticesColors(colors);
}

void Mesh::indices(const std::vector<uint16_t>& indices)
{
    m_impl->indices(indices);
}
