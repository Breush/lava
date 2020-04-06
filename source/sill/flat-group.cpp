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
        if (primitive->material() != nullptr) {
            // @fixme Well... this seems to work only for non-shared materials
            // and otherwise thanks to deallocator not doing anything...
            m_engine.scene2d().remove(*primitive->material());
        }
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
