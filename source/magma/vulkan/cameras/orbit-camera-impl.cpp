#include "./orbit-camera-impl.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <lava/chamber/logger.hpp>

#include "../buffer.hpp"
#include "../render-engine-impl.hpp"
#include "../user-data-render.hpp"

using namespace lava::magma;
using namespace lava::chamber;

OrbitCamera::Impl::Impl(RenderEngine& engine)
    : m_engine(engine.impl())
    , m_uniformBufferHolder({m_engine.device(), m_engine.commandPool()})
{
    // Create descriptor set
    VkDescriptorSetLayout layouts[] = {m_engine.cameraDescriptorSetLayout()};
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_engine.cameraDescriptorPool();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts;

    if (vkAllocateDescriptorSets(m_engine.device(), &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
        logger.error("magma.vulkan.camera") << "Failed to create descriptor set." << std::endl;
    }

    // Create uniform buffer
    m_uniformBufferHolder.create(vk::BufferUsageFlagBits::eUniformBuffer, sizeof(CameraUbo));

    // Set it up
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = m_uniformBufferHolder.buffer().castOld(); // @cleanup HPP
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(CameraUbo);

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = 0u;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.pImageInfo = nullptr;
    descriptorWrite.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(m_engine.device(), 1u, &descriptorWrite, 0, nullptr);

    updateBindings();
}

OrbitCamera::Impl::~Impl()
{
}

//----- ICamera

ICamera::UserData OrbitCamera::Impl::render(ICamera::UserData data)
{
    auto& userData = *reinterpret_cast<UserDataRenderIn*>(data);
    const auto& commandBuffer = *userData.commandBuffer;
    const auto& pipelineLayout = *userData.pipelineLayout;

    // Bind with the camera descriptor set
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, DESCRIPTOR_SET_INDEX, 1,
                            &m_descriptorSet, 0, nullptr);

    return nullptr;
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
    CameraUbo ubo = {};
    ubo.view = m_viewTransform;
    ubo.projection = m_projectionTransform;
    m_uniformBufferHolder.copy(ubo);
}
