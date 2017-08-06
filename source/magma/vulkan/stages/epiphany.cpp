#include "./epiphany.hpp"

#include <lava/chamber/logger.hpp>

#include "../buffer.hpp"
#include "../image.hpp"
#include "../render-engine-impl.hpp"
#include "../shader.hpp"
#include "../vertex.hpp"

namespace {
    struct CameraUbo {
        glm::mat4 invertedView;
        glm::mat4 invertedProjection;
        glm::vec4 wPosition;
    };

    struct LightUbo {
        glm::vec4 wPosition;
        float radius;
    };

    struct Vertex {
        glm::vec2 position;
    };
}

using namespace lava::magma;
using namespace lava::chamber;

Epiphany::Epiphany(RenderEngine::Impl& engine)
    : RenderStage(engine)
    , m_vertexShaderModule{engine.device()}
    , m_fragmentShaderModule{engine.device()}
    , m_descriptorPool{engine.device()}
    , m_descriptorSetLayout{engine.device()}
    , m_imageHolder{engine}
    , m_cameraBufferHolder(engine)
    , m_lightBufferHolder(engine)
    , m_framebuffer{engine.device()}
{
}

//----- RenderStage

void Epiphany::stageInit()
{
    logger.log() << "Initializing Epiphany Stage." << std::endl;
    logger.log().tab(1);

    //----- Shaders

    auto vertexShaderCode = vulkan::readGlslShaderFile("./data/shaders/stages/epiphany.vert");
    vulkan::createShaderModule(m_engine.device(), vertexShaderCode, m_vertexShaderModule);
    add({vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, m_vertexShaderModule, "main"});

    auto fragmentShaderCode = vulkan::readGlslShaderFile("./data/shaders/stages/epiphany-phong.frag");
    vulkan::createShaderModule(m_engine.device(), fragmentShaderCode, m_fragmentShaderModule);
    add({vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, m_fragmentShaderModule, "main"});

    //----- Descriptor pool

    // @todo Should we really have so many pools?
    // Maybe just one in a central place.

    std::array<vk::DescriptorPoolSize, 2> poolSizes;
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    poolSizes[0].descriptorCount = 2u;
    poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
    poolSizes[1].descriptorCount = 4u;

    vk::DescriptorPoolCreateInfo poolInfo;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 6u;

    if (m_engine.device().createDescriptorPool(&poolInfo, nullptr, m_descriptorPool.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.stages.present") << "Failed to create descriptor pool." << std::endl;
    }

    //----- Descriptor set layout

    vk::DescriptorSetLayoutBinding cameraUboLayoutBinding;
    cameraUboLayoutBinding.binding = 0;
    cameraUboLayoutBinding.descriptorCount = 1;
    cameraUboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    cameraUboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutBinding lightUboLayoutBinding;
    lightUboLayoutBinding.binding = 1;
    lightUboLayoutBinding.descriptorCount = 1;
    lightUboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    lightUboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutBinding normalLayoutBinding;
    normalLayoutBinding.binding = 2;
    normalLayoutBinding.descriptorCount = 1;
    normalLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    normalLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutBinding albedoLayoutBinding;
    albedoLayoutBinding.binding = 3;
    albedoLayoutBinding.descriptorCount = 1;
    albedoLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    albedoLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutBinding ormLayoutBinding;
    ormLayoutBinding.binding = 4;
    ormLayoutBinding.descriptorCount = 1;
    ormLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    ormLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutBinding depthLayoutBinding;
    depthLayoutBinding.binding = 5;
    depthLayoutBinding.descriptorCount = 1;
    depthLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    depthLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    std::array<vk::DescriptorSetLayoutBinding, 6> bindings = {cameraUboLayoutBinding, lightUboLayoutBinding, normalLayoutBinding,
                                                              albedoLayoutBinding,    ormLayoutBinding,      depthLayoutBinding};
    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = bindings.size();
    layoutInfo.pBindings = bindings.data();

    if (m_engine.device().createDescriptorSetLayout(&layoutInfo, nullptr, m_descriptorSetLayout.replace())
        != vk::Result::eSuccess) {
        logger.error("magma.vulkan.stages.present") << "Failed to create material descriptor set layout." << std::endl;
    }

    //----- Descriptor set

    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_descriptorSetLayout;

    if (m_engine.device().allocateDescriptorSets(&allocInfo, &m_descriptorSet) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.stages.present") << "Failed to create descriptor set." << std::endl;
    }

    add(m_descriptorSetLayout);

    //----- Uniform buffers

    // @todo Have vulkan::Ubo m_cameraUbo?
    m_cameraBufferHolder.create(vk::BufferUsageFlagBits::eUniformBuffer, sizeof(CameraUbo));
    m_lightBufferHolder.create(vk::BufferUsageFlagBits::eUniformBuffer, sizeof(LightUbo));

    // Set them up
    vk::DescriptorBufferInfo cameraBufferInfo;
    cameraBufferInfo.buffer = m_cameraBufferHolder.buffer();
    cameraBufferInfo.range = sizeof(CameraUbo);

    vk::DescriptorBufferInfo lightBufferInfo;
    lightBufferInfo.buffer = m_lightBufferHolder.buffer();
    lightBufferInfo.range = sizeof(LightUbo);

    std::array<vk::WriteDescriptorSet, 2> descriptorWrites;

    descriptorWrites[0].dstSet = m_descriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &cameraBufferInfo;

    descriptorWrites[1].dstSet = m_descriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].descriptorType = vk::DescriptorType::eUniformBuffer;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &lightBufferInfo;

    m_engine.device().updateDescriptorSets(descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

    //----- Attachments

    ColorAttachment targetColorAttachment;
    targetColorAttachment.format = vk::Format::eR8G8B8A8Unorm;
    targetColorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    add(targetColorAttachment);

    logger.log().tab(-1);
}

void Epiphany::stageUpdate()
{
    logger.log() << "Updating Epiphany stage." << std::endl;
    logger.log().tab(1);

    createResources();
    createFramebuffers();

    logger.log().tab(-1);
}

void Epiphany::stageRender(const vk::CommandBuffer& commandBuffer)
{
    //----- Prologue

    // Set render pass
    std::array<vk::ClearValue, 1> clearValues;
    // clearValues[0] does not matter

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_framebuffer;
    renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderPassInfo.renderArea.extent = m_extent;
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    // Light linked list
    fillLll();

    // Update UBOs
    updateUbos();

    //----- Render

    // Bind pipeline
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);

    // Draw
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
    commandBuffer.draw(6, 1, 0, 0);

    //----- Epilogue

    commandBuffer.endRenderPass();
}

//----- Internal

void Epiphany::createResources()
{
    // Target
    auto format = vk::Format::eR8G8B8A8Unorm;
    m_imageHolder.create(format, m_extent, vk::ImageAspectFlagBits::eColor);
}

void Epiphany::createFramebuffers()
{
    // Framebuffer
    vk::FramebufferCreateInfo framebufferInfo;
    framebufferInfo.renderPass = m_renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &m_imageHolder.view();
    framebufferInfo.width = m_extent.width;
    framebufferInfo.height = m_extent.height;
    framebufferInfo.layers = 1;

    if (m_engine.device().createFramebuffer(&framebufferInfo, nullptr, m_framebuffer.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.stages.g-buffer") << "Failed to create framebuffers." << std::endl;
    }
}

// @todo Have vulkan::DescriptorCombinedImageSampler
void Epiphany::normalImageView(const vk::ImageView& imageView, const vk::Sampler& sampler)
{
    vk::DescriptorImageInfo imageInfo;
    imageInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;

    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = 2;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    m_engine.device().updateDescriptorSets(1u, &descriptorWrite, 0, nullptr);
}

void Epiphany::albedoImageView(const vk::ImageView& imageView, const vk::Sampler& sampler)
{
    vk::DescriptorImageInfo imageInfo;
    imageInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;

    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = 3;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    m_engine.device().updateDescriptorSets(1u, &descriptorWrite, 0, nullptr);
}

void Epiphany::ormImageView(const vk::ImageView& imageView, const vk::Sampler& sampler)
{
    vk::DescriptorImageInfo imageInfo;
    imageInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;

    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = 4;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    m_engine.device().updateDescriptorSets(1u, &descriptorWrite, 0, nullptr);
}

void Epiphany::depthImageView(const vk::ImageView& imageView, const vk::Sampler& sampler)
{
    vk::DescriptorImageInfo imageInfo;
    // @note Correspond to the final layout specified at previous pass
    imageInfo.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;

    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = 5;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    m_engine.device().updateDescriptorSets(1u, &descriptorWrite, 0, nullptr);
}

void Epiphany::updateUbos()
{
    //----- Camera UBO

    if (m_engine.cameras().size() > 0) {
        const auto& camera = m_engine.camera(0);

        CameraUbo ubo;
        ubo.invertedView = glm::inverse(camera.viewTransform());
        ubo.invertedProjection = glm::inverse(camera.projectionTransform());
        ubo.wPosition = glm::vec4(camera.position(), 1.f);

        m_cameraBufferHolder.copy(ubo);
    }

    //----- Light UBO

    if (m_engine.pointLights().size() > 0) {
        const auto& pointLight = m_engine.pointLight(0);

        LightUbo ubo;
        ubo.wPosition = glm::vec4(pointLight.position(), 1.f);
        ubo.radius = pointLight.radius();

        m_lightBufferHolder.copy(ubo);
    }
}

void Epiphany::fillLll()
{
    // @todo Render a sphere for the light
}
