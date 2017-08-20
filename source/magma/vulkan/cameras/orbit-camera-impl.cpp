#include "./orbit-camera-impl.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <lava/chamber/logger.hpp>

#include "../render-scenes/render-scene-impl.hpp"
#include "../ubos.hpp"

using namespace lava::magma;
using namespace lava::chamber;

OrbitCamera::Impl::Impl(RenderScene& scene)
    : m_scene(scene.impl())
    , m_uboHolder(m_scene.engine())
{
}

OrbitCamera::Impl::~Impl()
{
}

//----- ICamera

void OrbitCamera::Impl::init()
{
    m_descriptorSet = m_scene.cameraDescriptorHolder().allocateSet();
    m_uboHolder.init(m_descriptorSet, {sizeof(vulkan::CameraUbo)});

    m_initialized = true;
    updateBindings();
}

void OrbitCamera::Impl::render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t descriptorSetIndex)
{
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, descriptorSetIndex, 1, &m_descriptorSet, 0,
                                     nullptr);
}

//----- OrbitCamera

void OrbitCamera::Impl::position(const glm::vec3& position)
{
    m_position = position;
    updateViewTransform();
}

void OrbitCamera::Impl::target(const glm::vec3& target)
{
    m_target = target;
    updateViewTransform();
}

void OrbitCamera::Impl::viewportRatio(float viewportRatio)
{
    m_viewportRatio = viewportRatio;
    updateProjectionTransform();
}

void OrbitCamera::Impl::updateViewTransform()
{
    // @todo Make up vector configurable?
    m_viewTransform = glm::lookAt(m_position, m_target, glm::vec3(0.f, 0.f, 1.f));
    updateBindings();
}

void OrbitCamera::Impl::updateProjectionTransform()
{
    // @todo FOV and clippings configurable?
    const auto nearClipping = 0.1f;
    const auto farClipping = 100.f;
    m_projectionTransform = glm::perspective(glm::radians(45.f), m_viewportRatio, nearClipping, farClipping);
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
    m_uboHolder.copy(0, ubo);
}
