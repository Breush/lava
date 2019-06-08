#include <lava/magma/lights/directional-light.hpp>

#include "../vulkan/lights/directional-light-impl.hpp"

using namespace lava::magma;

$pimpl_class(DirectionalLight, RenderScene&, scene);

$pimpl_method_const(DirectionalLight, RenderImage, shadowsRenderImage);

ILight::Impl& DirectionalLight::interfaceImpl()
{
    return *m_impl;
}

$pimpl_property(DirectionalLight, glm::vec3, direction);
