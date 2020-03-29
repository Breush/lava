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

    // @fixme Have a way to remove this callback when the component is destroyed.
    engine.onWindowExtentChanged([this](Extent2d extent) {
        m_updateDelay = 0.1f;
        m_extent = extent;
    });
}

CameraComponent::Impl::~Impl()
{
    auto& engine = m_entity.engine().impl();
    engine.scene().remove(*m_camera);
}

void CameraComponent::Impl::update(float dt)
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    // @note We delay updates as it might take a while
    // recontructing buffers and such.
    if (m_updateDelay > 0.f) {
        m_updateDelay -= dt;
        if (m_updateDelay <= 0.f) {
            m_camera->extent(m_extent);
            m_cameraController.updateCamera();
        }
    }
}

lava::Ray CameraComponent::Impl::coordinatesToRay(const glm::vec2& coordinates) const
{
    Ray ray;
    ray.origin = unproject(coordinates);
    ray.direction = glm::normalize(unproject(coordinates, 1.f) - ray.origin);
    return ray;
}

glm::vec3 CameraComponent::Impl::unproject(const glm::vec2& coordinates, float depth) const
{
    const auto& viewTransform = m_camera->viewTransform();
    const auto& projectionTransform = m_camera->projectionTransform();

    // @note Position is from (0, 0) top-left to (width, height) bottom-right
    // Camera final projection is from (-1, -1) top-left to (1, 1) bottom-right
    auto normalizedCoordinates = 2.f * coordinates / glm::vec2(m_extent.width, m_extent.height) - 1.f;

    // @fixme These could be cached inside m_camera, fact we shouldn't know camera
    // projection conventions either, so unproject could be a method of magma::OrbitCamera.
    auto viewTransformInverse = glm::inverse(viewTransform);
    auto projectionTransformInverse = glm::inverse(projectionTransform);

    auto localPosition = projectionTransformInverse * glm::vec4(normalizedCoordinates, depth, 1.f);
    auto position = glm::vec3(viewTransformInverse * (localPosition / localPosition.w));

    return position;
}

magma::Frustum CameraComponent::Impl::frustum(const glm::vec2& topLeftCoordinates, const glm::vec2& bottomRightCoordinates) const
{
    // See @note in unproject about normalized coordinates.
    auto topLeftRel = 2.f * topLeftCoordinates / glm::vec2(m_extent.width, m_extent.height) - 1.f;
    auto bottomRightRel = 2.f * bottomRightCoordinates / glm::vec2(m_extent.width, m_extent.height) - 1.f;
    return m_camera->frustum(topLeftRel, bottomRightRel);
}
