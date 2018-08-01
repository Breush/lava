#include <lava/magma/lights/point-light.hpp>

#include "../vulkan/lights/point-light-impl.hpp"

using namespace lava::magma;

$pimpl_class(PointLight, RenderScene&, scene);

ILight::Impl& PointLight::interfaceImpl()
{
    return *m_impl;
}

$pimpl_property(PointLight, glm::vec3, translation);
$pimpl_property_v(PointLight, float, radius);
