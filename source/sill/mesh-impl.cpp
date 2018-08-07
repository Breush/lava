#include "./mesh-impl.hpp"

#include "./mesh-primitive-impl.hpp"

using namespace lava::sill;

//----- Mesh primitives

void Mesh::Impl::primitives(std::vector<MeshPrimitive>&& primitives)
{
    m_primitives = std::move(primitives);
}

MeshPrimitive& Mesh::Impl::addPrimitive(GameEngine& engine)
{
    m_primitives.emplace_back(engine);
    return m_primitives.back();
}

//----- Internal interface

void Mesh::Impl::transform(const glm::mat4& transform)
{
    // @note Each magma::Mesh has its own transform,
    // there are no notions of primitive in magma.
    for (auto& primitive : m_primitives) {
        primitive.impl().magma()->transform(transform);
    }
}
