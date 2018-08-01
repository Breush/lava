#include "./point-light-impl.hpp"

using namespace lava::magma;

PointLight::Impl::Impl(RenderScene& scene)
    : m_scene(scene.impl())
{
}

void PointLight::Impl::init() {}
