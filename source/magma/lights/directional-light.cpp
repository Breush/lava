#include <lava/magma/lights/directional-light.hpp>

#include "../vulkan/lights/directional-light-impl.hpp"

using namespace lava::magma;

$pimpl_class(DirectionalLight, RenderScene&, scene);

ILight::Impl& DirectionalLight::interfaceImpl()
{
    return *m_impl;
}

$pimpl_property(DirectionalLight, glm::vec3, translation);
$pimpl_property(DirectionalLight, glm::vec3, direction);
