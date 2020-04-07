#include "./camera-component-impl.hpp"

#include "../game-engine-impl.hpp"

using namespace lava;
using namespace lava::sill;

CameraComponent::Impl::Impl(GameEntity& entity)
    : ComponentImpl(entity)
{
    // @fixme As with physics engine or font manager, we shouldn't need
    // to access impl() of engine here...
    auto& engine = m_entity.engine().impl();

    // @todo We might want to have a way to specify that this camera should be
    // using the window extent but user specific one. For instance, if the result
    // should be used as a mirror.
    m_camera = &engine.scene().make<magma::Camera>(engine.windowRenderTarget().extent());
    m_cameraController.bind(*m_camera);
    m_extent = engine.windowRenderTarget().extent();

    // @todo Let the viewport be configurable too...
    engine.renderEngine().addView(*m_camera, engine.windowRenderTarget(), Viewport{0, 0, 1, 1});

    m_onWindowExtentChangedId = engine.onWindowExtentChanged([this](Extent2d extent) {
        m_extent = extent;
        m_camera->extent(m_extent);
        m_cameraController.updateCamera();
    });
}

CameraComponent::Impl::~Impl()
{
    auto& engine = m_entity.engine().impl();
    engine.removeOnWindowExtentChanged(m_onWindowExtentChangedId);
    engine.scene().remove(*m_camera);
}

lava::Ray CameraComponent::Impl::coordinatesToRay(const glm::vec2& coordinates, float depth) const
{
    Ray ray;
    ray.origin = unproject(coordinates, depth);
    ray.direction = glm::normalize(unproject(coordinates, depth + 1.f) - ray.origin);
    return ray;
}

glm::vec3 CameraComponent::Impl::unproject(const glm::vec2& coordinates, float depth) const
{
    // @note Position is from (0, 0) top-left to (width, height) bottom-right
    // Camera final projection is from (-1, -1) top-left to (1, 1) bottom-right
    auto normalizedCoordinates = 2.f * coordinates / glm::vec2(m_extent.width, m_extent.height) - 1.f;
    auto localPosition = m_camera->projectionTransformInverse() * glm::vec4(normalizedCoordinates, depth, 1.f);
    auto position = glm::vec3(m_camera->viewTransformInverse() * (localPosition / localPosition.w));
    return position;
}

magma::Frustum CameraComponent::Impl::frustum(const glm::vec2& topLeftCoordinates, const glm::vec2& bottomRightCoordinates) const
{
    // See @note in unproject about normalized coordinates.
    auto topLeftRel = 2.f * topLeftCoordinates / glm::vec2(m_extent.width, m_extent.height) - 1.f;
    auto bottomRightRel = 2.f * bottomRightCoordinates / glm::vec2(m_extent.width, m_extent.height) - 1.f;
    return m_camera->frustum(topLeftRel, bottomRightRel);
}
