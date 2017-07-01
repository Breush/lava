#include "./point-light-impl.hpp"

using namespace lava::magma;

PointLight::Impl::Impl(RenderEngine& engine)
    : m_engine(engine.impl())
{
}
