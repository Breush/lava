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

    auto topLeftLocal = m_projectionMatrixInverse * glm::vec4(topLeftRel.x, topLeftRel.y, 0.f, 1.f);
    auto topRightLocal = m_projectionMatrixInverse * glm::vec4(bottomRightRel.x, topLeftRel.y, 0.f, 1.f);
    auto bottomLeftLocal = m_projectionMatrixInverse * glm::vec4(topLeftRel.x, bottomRightRel.y, 0.f, 1.f);
    auto bottomRightLocal = m_projectionMatrixInverse * glm::vec4(bottomRightRel.x, bottomRightRel.y, 0.f, 1.f);

    auto topLeft = glm::vec3(m_viewMatrixInverse * (topLeftLocal / topLeftLocal.w));
    auto topRight = glm::vec3(m_viewMatrixInverse * (topRightLocal / topRightLocal.w));
    auto bottomLeft = glm::vec3(m_viewMatrixInverse * (bottomLeftLocal / bottomLeftLocal.w));
    auto bottomRight = glm::vec3(m_viewMatrixInverse * (bottomRightLocal / bottomRightLocal.w));

    auto translation = glm::vec3(m_viewMatrixInverse[3]);

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

void Camera::viewTransform(const lava::Transform& viewTransform)
{
    m_viewTransform = viewTransform;
    m_viewMatrix = m_viewTransform.matrix();
    m_viewMatrixInverse = m_viewTransform.inverse().matrix();
    m_viewProjectionMatrixInverse = glm::inverse(m_projectionMatrix * m_viewMatrix);
    updateFrustum();

    // Update UBO
    // @fixme Now that we have the TRS info, this can be stored in one less member!
    auto transposeViewMatrix = glm::transpose(m_viewMatrix);
    m_ubo.viewTransform0 = transposeViewMatrix[0];
    m_ubo.viewTransform1 = transposeViewMatrix[1];
    m_ubo.viewTransform2 = transposeViewMatrix[2];
}

void Camera::projectionMatrix(const glm::mat4& projectionMatrix)
{
    m_projectionMatrix = projectionMatrix;
    m_projectionMatrixInverse = glm::inverse(m_projectionMatrix);
    m_viewProjectionMatrixInverse = glm::inverse(m_projectionMatrix * m_viewMatrix);
    updateFrustum();

    // Update UBO
    m_ubo.projectionFactors0[0] = m_projectionMatrix[0][0];
    m_ubo.projectionFactors0[1] = m_projectionMatrix[1][1];
    m_ubo.projectionFactors0[2] = m_projectionMatrix[2][2];
    m_ubo.projectionFactors0[3] = m_projectionMatrix[3][2];
    m_ubo.projectionFactors1[0] = m_projectionMatrix[2][0];
    m_ubo.projectionFactors1[1] = m_projectionMatrix[2][1];
}

// ----- Updates

void Camera::updateFrustum()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    m_frustum = frustum(glm::vec2{-1.f, -1.f}, glm::vec2{1.f, 1.f});
}
