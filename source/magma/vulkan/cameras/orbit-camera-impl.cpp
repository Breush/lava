#include "./orbit-camera-impl.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <lava/chamber/logger.hpp>

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
    m_viewTransform = glm::lookAt(m_translation, m_target, glm::vec3(0.f, 0.f, 1.f));
    updateBindings();
}

void OrbitCamera::Impl::updateProjectionTransform()
{
    const auto viewportRatio = static_cast<float>(m_extent.width) / static_cast<float>(m_extent.height);

    // @todo FOV and clippings configurable?
    const auto nearClipping = 0.1f;
    const auto farClipping = 100.f;
    m_projectionTransform = glm::perspective(glm::radians(45.f), viewportRatio, nearClipping, farClipping);
    m_projectionTransform[1][1] *= -1;

    updateBindings();
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
