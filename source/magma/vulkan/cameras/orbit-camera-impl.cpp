#include "./orbit-camera-impl.hpp"

#include "../render-scenes/render-scene-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

OrbitCamera::Impl::Impl(RenderScene& scene, Extent2d extent)
    : m_scene(scene.impl())
{
    this->extent(extent);
    updateProjectionTransform();
}

OrbitCamera::Impl::~Impl() {}

//----- ICamera

RenderImage OrbitCamera::Impl::renderImage() const
{
    return m_scene.cameraRenderImage(m_id);
}

RenderImage OrbitCamera::Impl::depthRenderImage() const
{
    return m_scene.cameraDepthRenderImage(m_id);
}

void OrbitCamera::Impl::polygonMode(PolygonMode polygonMode)
{
    m_polygonMode = polygonMode;

    if (m_initialized) {
        m_scene.updateCamera(m_id);
    }
}

//----- ICamera::Impl

void OrbitCamera::Impl::init(uint32_t id)
{
    m_id = id;

    m_initialized = true;
    updateBindings();
}

void OrbitCamera::Impl::render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                               uint32_t pushConstantOffset) const
{
    commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                                pushConstantOffset, sizeof(vulkan::CameraUbo), &m_ubo);
}

void OrbitCamera::Impl::extent(Extent2d extent)
{
    // Ignoring resize with same extent
    if (m_extent.width == extent.width && m_extent.height == extent.height) {
        return;
    }

    m_extent.width = extent.width;
    m_extent.height = extent.height;
    updateProjectionTransform();

    if (m_initialized) {
        m_scene.updateCamera(m_id);
    }
}

//----- OrbitCamera

void OrbitCamera::Impl::translation(const glm::vec3& translation)
{
    m_translation = translation;
    updateViewTransform();
}

void OrbitCamera::Impl::target(const glm::vec3& target)
{
    m_target = target;
    updateViewTransform();
}

//----- Private

void OrbitCamera::Impl::updateViewTransform()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    // @todo Make up vector configurable?
    m_viewTransform = glm::lookAtRH(m_translation, m_target, glm::vec3(0.f, 0.f, 1.f));

    m_inverseViewProjectionTransform = glm::inverse(m_projectionTransform * m_viewTransform);

    updateFrustum();
    updateBindings();
}

void OrbitCamera::Impl::updateProjectionTransform()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    const auto aspectRatio = static_cast<float>(m_extent.width) / static_cast<float>(m_extent.height);

    // @todo FOV and clippings configurable?
    const auto FOVy = glm::radians(45.f);

    m_projectionTransform = glm::perspectiveRH(FOVy, aspectRatio, m_nearClip, m_farClip);
    m_projectionTransform[1][1] *= -1;

    m_inverseViewProjectionTransform = glm::inverse(m_projectionTransform * m_viewTransform);

    updateFrustum();
    updateBindings();
}

void OrbitCamera::Impl::updateFrustum()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

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

    // Left plane
    m_frustum.leftNormal = glm::normalize(glm::cross(topLeft - m_translation, bottomLeft - m_translation));
    m_frustum.leftDistance = glm::dot(m_translation, m_frustum.leftNormal);

    // Right plane
    m_frustum.rightNormal = glm::normalize(glm::cross(bottomRight - m_translation, topRight - m_translation));
    m_frustum.rightDistance = glm::dot(m_translation, m_frustum.rightNormal);

    // Bottom plane
    m_frustum.bottomNormal = glm::normalize(glm::cross(bottomLeft - m_translation, bottomRight - m_translation));
    m_frustum.bottomDistance = glm::dot(m_translation, m_frustum.bottomNormal);

    // Top plane
    m_frustum.topNormal = glm::normalize(glm::cross(topRight - m_translation, topLeft - m_translation));
    m_frustum.topDistance = glm::dot(m_translation, m_frustum.topNormal);

    // Forward
    auto forwardNormal = glm::normalize(m_target - m_translation);
    auto cameraDistance = glm::dot(m_translation, forwardNormal);
    m_frustum.forward = forwardNormal;
    m_frustum.near = cameraDistance + m_nearClip;
    m_frustum.far = cameraDistance + m_farClip;
}

void OrbitCamera::Impl::updateBindings()
{
    if (!m_initialized) return;

    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    auto transposeViewTransform = glm::transpose(m_viewTransform);
    m_ubo.viewTransform0 = transposeViewTransform[0];
    m_ubo.viewTransform1 = transposeViewTransform[1];
    m_ubo.viewTransform2 = transposeViewTransform[2];

    m_ubo.projectionFactors0[0] = m_projectionTransform[0][0];
    m_ubo.projectionFactors0[1] = m_projectionTransform[1][1];
    m_ubo.projectionFactors0[2] = m_projectionTransform[2][2];
    m_ubo.projectionFactors0[3] = m_projectionTransform[3][2];
    m_ubo.projectionFactors1[0] = m_projectionTransform[2][0];
    m_ubo.projectionFactors1[1] = m_projectionTransform[2][1];
    m_ubo.projectionFactors1[2] = m_extent.width;
    m_ubo.projectionFactors1[3] = m_extent.height;
}
