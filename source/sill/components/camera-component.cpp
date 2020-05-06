#include <lava/sill/components/camera-component.hpp>

#include <lava/sill/game-engine.hpp>
#include <lava/sill/game-entity.hpp>

using namespace lava;
using namespace lava::sill;

CameraComponent::CameraComponent(GameEntity& entity)
    : IComponent(entity)
{
    auto& engine = m_entity.engine();

    // @todo :CameraConfiguration We might want to have a way to specify that this camera should be
    // using the window extent but user specific one. For instance, if the result
    // should be used as a mirror.
    m_camera = &engine.scene().make<magma::Camera>(engine.windowRenderTarget().extent());
    m_cameraController.bind(*m_camera);
    m_extent = engine.windowRenderTarget().extent();

    // @todo :CameraConfiguration Let the viewport be configurable too...
    engine.renderEngine().addView(*m_camera, engine.windowRenderTarget(), Viewport{0, 0, 1, 1});

    m_onWindowExtentChangedId = engine.onWindowExtentChanged([this](Extent2d extent) {
        m_extent = extent;
        m_camera->extent(m_extent);
        m_cameraController.updateCamera();
    });
}

CameraComponent::~CameraComponent()
{
    auto& engine = m_entity.engine();
    engine.removeOnWindowExtentChanged(m_onWindowExtentChangedId);
    engine.scene().remove(*m_camera);
}

// ----- Controls

void CameraComponent::go(const glm::vec3& origin)
{
    auto offset = origin - m_cameraController.origin();
    auto target = m_cameraController.target() + offset;
    m_cameraController.origin(origin);
    m_cameraController.target(target);
}

void CameraComponent::goForward(float distance, const glm::vec3& constraints)
{
    auto offset = distance * glm::normalize(constraints * (target() - origin()));
    m_cameraController.origin(m_cameraController.origin() + offset);
    m_cameraController.target(m_cameraController.target() + offset);
}

void CameraComponent::goRight(float distance, const glm::vec3& constraints)
{
    auto offset = distance * glm::normalize(constraints * glm::cross(target() - origin(), {0, 0, 1}));
    m_cameraController.origin(m_cameraController.origin() + offset);
    m_cameraController.target(m_cameraController.target() + offset);
}

// ----- Transforms

glm::vec3 CameraComponent::unproject(const glm::vec2& coordinates, float depth) const
{
    // @note Position is from (0, 0) top-left to (width, height) bottom-right
    // Camera final projection is from (-1, -1) top-left to (1, 1) bottom-right
    auto normalizedCoordinates = 2.f * coordinates / glm::vec2(m_extent.width, m_extent.height) - 1.f;
    auto localPosition = m_camera->projectionMatrixInverse() * glm::vec4(normalizedCoordinates, depth, 1.f);
    auto position = glm::vec3(m_camera->viewMatrixInverse() * (localPosition / localPosition.w));
    return position;
}

Ray CameraComponent::unprojectAsRay(const glm::vec2& coordinates, float depth) const
{
    Ray ray;
    ray.origin = unproject(coordinates, depth);
    ray.direction = glm::normalize(unproject(coordinates, depth + 1.f) - ray.origin);
    return ray;
}

lava::Transform CameraComponent::unprojectAsTransform(const glm::vec2& coordinates, float depth) const
{
    lava::Transform transform;
    transform.translation = unproject(coordinates, depth);
    transform.rotation = glm::inverse(m_camera->viewTransform().rotation);
    return transform;
}

magma::Frustum CameraComponent::unprojectAsFrustum(const glm::vec2& topLeftCoordinates, const glm::vec2& bottomRightCoordinates) const
{
    // See @note in unproject about normalized coordinates.
    auto topLeftRel = 2.f * topLeftCoordinates / glm::vec2(m_extent.width, m_extent.height) - 1.f;
    auto bottomRightRel = 2.f * bottomRightCoordinates / glm::vec2(m_extent.width, m_extent.height) - 1.f;
    return m_camera->frustum(topLeftRel, bottomRightRel);
}
