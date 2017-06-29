#include <lava/magma/cameras/orbit-camera.hpp>

#include <lava/chamber/pimpl.hpp>

#include "../vulkan/cameras/orbit-camera-impl.hpp"

using namespace lava;

$pimpl_class(OrbitCamera, RenderEngine&, engine);

$pimpl_method(OrbitCamera, void, position, const glm::vec3&, position);
$pimpl_method(OrbitCamera, void, target, const glm::vec3&, target);
