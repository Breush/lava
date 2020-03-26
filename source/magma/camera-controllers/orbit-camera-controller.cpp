#include <lava/magma/camera-controllers/orbit-camera-controller.hpp>

#include <lava/magma/camera.hpp>

using namespace lava::chamber;
using namespace lava::magma;

void OrbitCameraController::bind(Camera& camera)
{
    m_camera = &camera;

    updateCameraViewTransform();
    updateCameraProjectionTransform();
}

void OrbitCameraController::updateCamera()
{
    updateCameraProjectionTransform();
}

// ----- Controls

void OrbitCameraController::origin(const glm::vec3& origin)
{
    m_origin = origin;
    updateCameraViewTransform();
}

void OrbitCameraController::target(const glm::vec3& target)
{
    m_target = target;
    updateCameraViewTransform();
}

void OrbitCameraController::radius(float radius)
{
    origin(m_target + glm::normalize(m_origin - m_target) * radius);
}

void OrbitCameraController::strafe(float x, float y)
{
    auto viewDirection = m_target - m_origin;
    auto radius = glm::length(viewDirection);

    auto normal = glm::normalize(viewDirection);
    auto tangent = glm::cross(m_up, normal);
    tangent = glm::normalize(tangent - normal * glm::dot(normal, tangent)); // Orthogonalization
    auto bitangent = glm::normalize(glm::cross(normal, tangent));

    glm::vec3 delta = (x * tangent + y * bitangent) * radius;
    origin(m_origin + delta);
    target(m_target + delta);
}

void OrbitCameraController::orbitAdd(float longitudeAngle, float latitudeAngle)
{
    auto relativePosition = m_origin - m_target;
    auto axis = glm::vec3(relativePosition.y, -relativePosition.x, 0);

    auto currentLatitudeAngle = std::asin(relativePosition.z / glm::length(relativePosition));
    if (currentLatitudeAngle + latitudeAngle > math::PI_OVER_TWO - 0.01) {
        latitudeAngle = math::PI_OVER_TWO - 0.01 - currentLatitudeAngle;
    }
    else if (currentLatitudeAngle + latitudeAngle < -math::PI_OVER_TWO + 0.01) {
        latitudeAngle = -math::PI_OVER_TWO + 0.01 - currentLatitudeAngle;
    }

    auto longitudeDelta = glm::rotateZ(relativePosition, longitudeAngle) - relativePosition;
    auto latitudeDelta = glm::rotate(relativePosition, latitudeAngle, axis) - relativePosition;
    origin(m_origin + longitudeDelta + latitudeDelta);
}

void OrbitCameraController::rotateAtOrigin(float longitudeAngle, float latitudeAngle)
{
    auto relativePosition = m_target - m_origin;
    auto axis = glm::vec3(relativePosition.y, -relativePosition.x, 0);

    auto currentLatitudeAngle = std::asin(relativePosition.z / glm::length(relativePosition));
    if (currentLatitudeAngle + latitudeAngle > math::PI_OVER_TWO - 0.01) {
        latitudeAngle = math::PI_OVER_TWO - 0.01 - currentLatitudeAngle;
    }
    else if (currentLatitudeAngle + latitudeAngle < -math::PI_OVER_TWO + 0.01) {
        latitudeAngle = -math::PI_OVER_TWO + 0.01 - currentLatitudeAngle;
    }

    auto longitudeDelta = glm::rotateZ(relativePosition, longitudeAngle) - relativePosition;
    auto latitudeDelta = glm::rotate(relativePosition, latitudeAngle, axis) - relativePosition;
    target(m_target + longitudeDelta + latitudeDelta);
}

// ----- Updates

void OrbitCameraController::updateCameraViewTransform()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    m_camera->viewTransform(glm::lookAtRH(m_origin, m_target, m_up));
}

void OrbitCameraController::updateCameraProjectionTransform()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    const auto aspectRatio = static_cast<float>(m_camera->extent().width) / static_cast<float>(m_camera->extent().height);

    auto projectionTransform = glm::perspectiveRH(m_camera->fovY(), aspectRatio, m_camera->nearClip(), m_camera->farClip());
    projectionTransform[1][1] *= -1;

    m_camera->projectionTransform(projectionTransform);
}
