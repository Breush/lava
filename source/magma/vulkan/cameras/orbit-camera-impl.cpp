#include "./orbit-camera-impl.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace lava;

OrbitCamera::Impl::Impl(RenderEngine& engine)
    : m_engine(engine.impl())
{
}

OrbitCamera::Impl::~Impl()
{
}

void OrbitCamera::Impl::position(const glm::vec3& position)
{
    m_position = position;
    updateViewTransform();
}

void OrbitCamera::Impl::target(const glm::vec3& target)
{
    m_target = target;
    updateViewTransform();
}

void OrbitCamera::Impl::viewportRatio(float viewportRatio)
{
    m_viewportRatio = viewportRatio;
    updateProjectionTransform();
}

// @todo These could be implemented in OrbitCamera itself
void OrbitCamera::Impl::latitudeAdd(float latitudeDelta)
{
    auto relativePosition = m_position - m_target;
    auto axis = glm::vec3(-relativePosition.y, relativePosition.x, 0);
    m_position = m_target + glm::rotate(relativePosition, latitudeDelta, axis);
    updateViewTransform();
}

void OrbitCamera::Impl::longitudeAdd(float longitudeDelta)
{
    m_position = m_target + glm::rotateZ(m_position - m_target, longitudeDelta);
    updateViewTransform();
}

void OrbitCamera::Impl::updateViewTransform()
{
    // @todo Make up vector configurable?
    m_viewTransform = glm::lookAt(m_position, m_target, glm::vec3(0.f, 0.f, 1.f));
}

void OrbitCamera::Impl::updateProjectionTransform()
{
    // @todo FOV configurable?
    m_projectionTransform = glm::perspective(glm::radians(45.f), m_viewportRatio, 0.1f, 10.f);
    m_projectionTransform[1][1] *= -1;
}
