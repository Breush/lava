#include <lava/sill/flat-group.hpp>

#include <lava/sill/game-engine.hpp>

using namespace lava;
using namespace lava::sill;

FlatGroup::FlatGroup(GameEngine& engine)
    : m_engine(engine)
{
}

FlatGroup::~FlatGroup()
{
    for (auto& primitive : m_primitives) {
        m_engine.scene2d().remove(*primitive);
    }
}

//----- Primitives

magma::Flat& FlatGroup::addPrimitive()
{
    auto& primitive = m_engine.scene2d().make<magma::Flat>();
    m_primitives.emplace_back(&primitive);
    return primitive;
}

//----- Transforms

void FlatGroup::transform(const glm::mat4& transform)
{
    // @note Each magma::Flat has its own transform,
    // there are no notions of primitive in magma.
    for (auto& primitive : m_primitives) {
        primitive->transform(transform);
    }
}
