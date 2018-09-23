#include "./orbit-camera-impl.hpp"

#include "../render-scenes/render-scene-impl.hpp"
#include "../ubos.hpp"

using namespace lava::magma;
using namespace lava::chamber;

OrbitCamera::Impl::Impl(RenderScene& scene, Extent2d extent)
    : m_scene(scene.impl())
    , m_uboHolder(m_scene.engine())
{
    this->extent(extent);
    updateProjectionTransform();
}

OrbitCamera::Impl::~Impl()
{
    if (m_initialized) {
        m_scene.cameraDescriptorHolder().freeSet(m_descriptorSet);
    }
}

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
    m_descriptorSet = m_scene.cameraDescriptorHolder().allocateSet("orbit-camera." + std::to_string(id));
    m_uboHolder.init(m_descriptorSet, m_scene.cameraDescriptorHolder().uniformBufferBindingOffset(), {sizeof(vulkan::CameraUbo)});

    m_initialized = true;
    updateBindings();
}

void OrbitCamera::Impl::render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                               uint32_t descriptorSetIndex) const
{
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, descriptorSetIndex, 1, &m_descriptorSet, 0,
                                     nullptr);
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

void OrbitCamera::Impl::updateViewTransform()
{
    // @todo Make up vector configurable?
    m_viewTransform = glm::lookAtRH(m_translation, m_target, glm::vec3(0.f, 0.f, 1.f));

    updateFrustum();
    updateBindings();
}

void OrbitCamera::Impl::updateProjectionTransform()
{
    const auto aspectRatio = static_cast<float>(m_extent.width) / static_cast<float>(m_extent.height);

    // @todo FOV and clippings configurable?
    const auto n = 0.1f;
    const auto f = 100.f;
    const auto FOVy = glm::radians(45.f);

    m_projectionTransform = glm::perspectiveRH(FOVy, aspectRatio, n, f);
    m_projectionTransform[1][1] *= -1;

    updateFrustum();
    updateBindings();
}

void OrbitCamera::Impl::updateFrustum()
{
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
    const auto n = 0.1f;
    const auto f = 100.f;
    auto forwardNormal = glm::normalize(m_target - m_translation);
    auto cameraDistance = glm::dot(m_translation, forwardNormal);
    m_frustum.forward = forwardNormal;
    m_frustum.near = cameraDistance + n;
    m_frustum.far = cameraDistance + f;
}

//----- Private

void OrbitCamera::Impl::updateBindings()
{
    if (!m_initialized) return;

    vulkan::CameraUbo ubo;
    ubo.view = m_viewTransform;
    ubo.projection = m_projectionTransform;
    ubo.wPosition = glm::vec4(m_translation, 1.f);
    ubo.extent = glm::uvec2(m_extent.width, m_extent.height);
    m_uboHolder.copy(0, ubo);
}
