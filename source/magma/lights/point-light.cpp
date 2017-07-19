#include <lava/magma/lights/point-light.hpp>

#include "./point-light-impl.hpp"

using namespace lava::magma;

$pimpl_class(PointLight, RenderEngine&, engine);

// IPointLight
$pimpl_method_const(PointLight, const glm::vec3&, position);
$pimpl_method_const(PointLight, float, radius);

$pimpl_method(PointLight, void, position, const glm::vec3&, position);
$pimpl_method(PointLight, void, radius, float, radius);
