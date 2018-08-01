#include "./directional-light-impl.hpp"

using namespace lava::magma;

DirectionalLight::Impl::Impl(RenderScene& scene)
    : m_scene(scene.impl())
{
}

void DirectionalLight::Impl::init() {}
