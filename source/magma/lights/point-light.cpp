#include <lava/magma/lights/point-light.hpp>

#include "./point-light-impl.hpp"

using namespace lava::magma;

$pimpl_class(PointLight, RenderScene&, scene);

// IPointLight
$pimpl_method(PointLight, void, init);

$pimpl_property(PointLight, glm::vec3, position);
$pimpl_property_v(PointLight, float, radius);
