#include <lava/magma/camera.hpp>

#include "./aft-vulkan/camera-aft.hpp"

using namespace lava::magma;

Camera::Camera(Scene& scene, Extent2d extent)
    : m_scene(scene)
    , m_extent(extent)
{
    new (&aft()) CameraAft(*this, m_scene);

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

Frustum Camera::frustum(const glm::vec2& topLeftRel, const glm::vec2& bottomRightRel) const
{
    Frustum frustum;

    auto topLeftLocal = m_projectionTransformInverse * glm::vec4(topLeftRel.x, topLeftRel.y, 0.f, 1.f);
    auto topRightLocal = m_projectionTransformInverse * glm::vec4(bottomRightRel.x, topLeftRel.y, 0.f, 1.f);
    auto bottomLeftLocal = m_projectionTransformInverse * glm::vec4(topLeftRel.x, bottomRightRel.y, 0.f, 1.f);
    auto bottomRightLocal = m_projectionTransformInverse * glm::vec4(bottomRightRel.x, bottomRightRel.y, 0.f, 1.f);

    auto topLeft = glm::vec3(m_viewTransformInverse * (topLeftLocal / topLeftLocal.w));
    auto topRight = glm::vec3(m_viewTransformInverse * (topRightLocal / topRightLocal.w));
    auto bottomLeft = glm::vec3(m_viewTransformInverse * (bottomLeftLocal / bottomLeftLocal.w));
    auto bottomRight = glm::vec3(m_viewTransformInverse * (bottomRightLocal / bottomRightLocal.w));

    auto translation = glm::vec3(m_viewTransformInverse[3]);

    // Forward
    frustum.forward = glm::normalize(glm::cross(topLeft - bottomLeft, bottomRight - bottomLeft));

    // Left plane
    frustum.leftNormal = glm::normalize(glm::cross(topLeft - translation, bottomLeft - translation));
    frustum.leftDistance = glm::dot(translation, frustum.leftNormal);

    // Right plane
    frustum.rightNormal = glm::normalize(glm::cross(bottomRight - translation, topRight - translation));
    frustum.rightDistance = glm::dot(translation, frustum.rightNormal);

    // Bottom plane
    frustum.bottomNormal = glm::normalize(glm::cross(bottomLeft - translation, bottomRight - translation));
    frustum.bottomDistance = glm::dot(translation, frustum.bottomNormal);

    // Top plane
    frustum.topNormal = glm::normalize(glm::cross(topRight - translation, topLeft - translation));
    frustum.topDistance = glm::dot(translation, frustum.topNormal);

    // Forward
    auto cameraDistance = glm::dot(translation, frustum.forward);
    frustum.near = cameraDistance + m_nearClip;
    frustum.far = cameraDistance + m_farClip;

    return frustum;
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
    m_viewTransformInverse = glm::inverse(m_viewTransform);
    m_viewProjectionTransformInverse = glm::inverse(m_projectionTransform * m_viewTransform);
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
    m_projectionTransformInverse = glm::inverse(m_projectionTransform);
    m_viewProjectionTransformInverse = glm::inverse(m_projectionTransform * m_viewTransform);
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

    m_frustum = frustum(glm::vec2{-1.f, -1.f}, glm::vec2{1.f, 1.f});
}
