#include <lava/magma/cameras/orbit-camera.hpp>

// #include <lava/chamber/pimpl.hpp>
// #include "../vulkan/cameras/orbit-camera.hpp"

using namespace lava;

// @todo pimplify
// $pimpl_class(OrbitCamera, RenderEngine&, engine);

OrbitCamera::OrbitCamera(RenderEngine& engine)
{
}

OrbitCamera::~OrbitCamera()
{
}

void OrbitCamera::position(const glm::vec3& position)
{
    // m_impl->position(position);
}

void OrbitCamera::target(const glm::vec3& target)
{
    // m_impl->target(position);
}
