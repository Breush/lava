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

void Mesh::vertices(const std::vector<glm::vec2>& vertices)
{
    m_impl->vertices(vertices);
}

void Mesh::indices(const std::vector<uint16_t>& indices)
{
    m_impl->indices(indices);
}
