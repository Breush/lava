#include <lava/magma/cameras/orbit-camera.hpp>

#include <lava/chamber/pimpl.hpp>

#include "../vulkan/cameras/orbit-camera-impl.hpp"

using namespace lava;

$pimpl_class(OrbitCamera, RenderEngine&, engine);

// ICamera
$pimpl_method_const(OrbitCamera, const glm::vec3&, position);
$pimpl_method_const(OrbitCamera, const glm::mat4&, viewTransform);
$pimpl_method_const(OrbitCamera, const glm::mat4&, projectionTransform);

$pimpl_method(OrbitCamera, void, position, const glm::vec3&, position);
$pimpl_method(OrbitCamera, void, target, const glm::vec3&, target);
$pimpl_method(OrbitCamera, void, viewportRatio, float, viewportRatio);
