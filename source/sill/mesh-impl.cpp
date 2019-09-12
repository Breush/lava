#include "./mesh-impl.hpp"

#include <lava/sill/game-engine.hpp>

using namespace lava;
using namespace lava::sill;

Mesh::Impl::Impl(GameEngine& engine)
    : m_engine(engine)
{
}

Mesh::Impl::~Impl()
{
    for (auto& primitive : m_primitives) {
        if (primitive->material() != nullptr) {
            m_engine.scene().remove(*primitive->material());
        }
        m_engine.scene().remove(*primitive);
    }
}

//----- Mesh primitives

magma::Mesh& Mesh::Impl::addPrimitive()
{
    auto& mesh = m_engine.scene().make<magma::Mesh>();
    m_primitives.emplace_back(&mesh);
    return mesh;
}

//----- Internal interface

void Mesh::Impl::transform(const glm::mat4& transform)
{
    // @note Each magma::Mesh has its own transform,
    // there are no notions of primitive in magma.
    for (auto& primitive : m_primitives) {
        primitive->transform(transform);
    }
}
