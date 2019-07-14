#include <lava/magma/camera.hpp>

#include "./aft-vulkan/camera-aft.hpp"

using namespace lava::magma;

Camera::Camera(RenderScene& scene, Extent2d extent)
    : m_scene(scene)
    , m_extent(extent)
{
    new (&aft()) CameraAft(*this, m_scene.impl());

    updateFrustum();

    // Update UBO
    m_ubo.projectionFactors1[2] = m_extent.width;
    m_ubo.projectionFactors1[3] = m_extent.height;
}

Camera::~Camera()
{
    aft().~CameraAft();
}

// ----- Rendering

RenderImage Camera::renderImage() const
{
    return aft().foreRenderImage();
}

RenderImage Camera::depthRenderImage() const
{
    return aft().foreDepthRenderImage();
}

void Camera::polygonMode(PolygonMode polygonMode)
{
    m_polygonMode = polygonMode;
    aft().forePolygonModeChanged();
}

// ----- Init-time configuration

void Camera::extent(Extent2d extent)
{
    // Ignoring resize with same extent
    if (m_extent == extent) return;

    m_extent = extent;
    aft().foreExtentChanged();

    // Update UBO
    m_ubo.projectionFactors1[2] = m_extent.width;
    m_ubo.projectionFactors1[3] = m_extent.height;
}

// ----- Transforms

void Camera::viewTransform(const glm::mat4& viewTransform)
{
    m_viewTransform = viewTransform;
    m_inverseViewProjectionTransform = glm::inverse(m_projectionTransform * m_viewTransform);
    updateFrustum();

    // Update UBO
    auto transposeViewTransform = glm::transpose(m_viewTransform);
    m_ubo.viewTransform0 = transposeViewTransform[0];
    m_ubo.viewTransform1 = transposeViewTransform[1];
    m_ubo.viewTransform2 = transposeViewTransform[2];
}

void Camera::projectionTransform(const glm::mat4& projectionTransform)
{
    m_projectionTransform = projectionTransform;
    m_inverseViewProjectionTransform = glm::inverse(m_projectionTransform * m_viewTransform);
    updateFrustum();

    // Update UBO
    m_ubo.projectionFactors0[0] = m_projectionTransform[0][0];
    m_ubo.projectionFactors0[1] = m_projectionTransform[1][1];
    m_ubo.projectionFactors0[2] = m_projectionTransform[2][2];
    m_ubo.projectionFactors0[3] = m_projectionTransform[3][2];
    m_ubo.projectionFactors1[0] = m_projectionTransform[2][0];
    m_ubo.projectionFactors1[1] = m_projectionTransform[2][1];
}

// ----- Updates

void Camera::updateFrustum()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    // @fixme These inverses could be precomputed, as they are needed in sill::CameraComponent::Impl too.
    auto viewTransformInverse = glm::inverse(m_viewTransform);
    auto projectionTransformInverse = glm::inverse(m_projectionTransform);

    auto topLeftLocal = projectionTransformInverse * glm::vec4(-1, -1, 0, 1);
    auto topRightLocal = projectionTransformInverse * glm::vec4(1, -1, 0, 1);
    auto bottomLeftLocal = projectionTransformInverse * glm::vec4(-1, 1, 0, 1);
    auto bottomRightLocal = projectionTransformInverse * glm::vec4(1, 1, 0, 1);

    auto topLeft = glm::vec3(viewTransformInverse * (topLeftLocal / topLeftLocal.w));
    auto topRight = glm::vec3(viewTransformInverse * (topRightLocal / topRightLocal.w));
    auto bottomLeft = glm::vec3(viewTransformInverse * (bottomLeftLocal / bottomLeftLocal.w));
    auto bottomRight = glm::vec3(viewTransformInverse * (bottomRightLocal / bottomRightLocal.w));

    auto translation = glm::vec3(viewTransformInverse[3]);

    // Forward
    m_frustum.forward = glm::normalize(glm::cross(topLeft - bottomLeft, bottomRight - bottomLeft));

    // Left plane
    m_frustum.leftNormal = glm::normalize(glm::cross(topLeft - translation, bottomLeft - translation));
    m_frustum.leftDistance = glm::dot(translation, m_frustum.leftNormal);

    // Right plane
    m_frustum.rightNormal = glm::normalize(glm::cross(bottomRight - translation, topRight - translation));
    m_frustum.rightDistance = glm::dot(translation, m_frustum.rightNormal);

    // Bottom plane
    m_frustum.bottomNormal = glm::normalize(glm::cross(bottomLeft - translation, bottomRight - translation));
    m_frustum.bottomDistance = glm::dot(translation, m_frustum.bottomNormal);

    // Top plane
    m_frustum.topNormal = glm::normalize(glm::cross(topRight - translation, topLeft - translation));
    m_frustum.topDistance = glm::dot(translation, m_frustum.topNormal);

    // Forward
    auto cameraDistance = glm::dot(translation, m_frustum.forward);
    m_frustum.near = cameraDistance + m_nearClip;
    m_frustum.far = cameraDistance + m_farClip;
}
