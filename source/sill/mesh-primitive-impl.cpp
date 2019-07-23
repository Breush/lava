#include "./mesh-primitive-impl.hpp"

#include "./game-engine-impl.hpp"

using namespace lava::sill;

MeshPrimitive::Impl::Impl(GameEngine& engine)
    : m_engine(engine.impl())
{
    m_magma = &m_engine.scene().make<magma::Mesh>();
}

MeshPrimitive::Impl::~Impl()
{
    m_engine.scene().remove(*m_magma);
    m_magma = nullptr;
}
