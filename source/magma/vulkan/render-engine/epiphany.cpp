#include "./epiphany.hpp"

#include <lava/chamber/logger.hpp>

#include "../buffer.hpp"
#include "../image.hpp"
#include "../render-engine-impl.hpp"
#include "../shader.hpp"
#include "../vertex.hpp"

namespace {
    struct CameraUbo {
        glm::vec3 wPosition;
    };

    struct LightUbo {
        glm::vec3 wPosition;
    };

    struct Vertex {
        glm::vec2 position;
    };
}

using namespace lava::magma;
using namespace lava::chamber;

Epiphany::Epiphany(RenderEngine::Impl& engine)
    : IStage(engine)
    , m_vertShaderModule{m_engine.device().vk()}
    , m_fragShaderModule{m_engine.device().vk()}
    , m_descriptorPool{m_engine.device().vk()}
    , m_descriptorSetLayout{m_engine.device().vk()}
    , m_imageHolder{m_engine.device()}
    , m_framebuffer{m_engine.device().vk()}
    , m_cameraUniformStagingBuffer{m_engine.device().vk()}
    , m_cameraUniformStagingBufferMemory{m_engine.device().vk()}
    , m_cameraUniformBuffer{m_engine.device().vk()}
    , m_cameraUniformBufferMemory{m_engine.device().vk()}
    , m_lightUniformStagingBuffer{m_engine.device().vk()}
    , m_lightUniformStagingBufferMemory{m_engine.device().vk()}
    , m_lightUniformBuffer{m_engine.device().vk()}
    , m_lightUniformBufferMemory{m_engine.device().vk()}
{
}

//----- IStage

void Epiphany::init()
{
    logger.log() << "Initializing Epiphany Stage." << std::endl;
    logger.log().tab(1);

    // @cleanup HPP
    const auto& vk_device = m_engine.device().vk();

    //----- Shaders

    auto vertShaderCode = vulkan::readGlslShaderFile("./data/shaders/render-engine/epiphany.vert");
    auto fragShaderCode = vulkan::readGlslShaderFile("./data/shaders/render-engine/epiphany-phong.frag");

    vulkan::createShaderModule(vk_device, vertShaderCode, m_vertShaderModule);
    vulkan::createShaderModule(vk_device, fragShaderCode, m_fragShaderModule);

    //----- Descriptor pool

    // @todo Should we really have so many pools?
    // Maybe just one in a central place.

    std::array<vk::DescriptorPoolSize, 2> poolSizes;
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    poolSizes[0].descriptorCount = 2u;
    poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
    poolSizes[1].descriptorCount = 2u;

    vk::DescriptorPoolCreateInfo poolInfo;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 4u;

    if (vk_device.createDescriptorPool(&poolInfo, nullptr, m_descriptorPool.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.present") << "Failed to create descriptor pool." << std::endl;
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

    std::array<vk::DescriptorSetLayoutBinding, 4> bindings = {cameraUboLayoutBinding, lightUboLayoutBinding, normalLayoutBinding,
                                                              albedoLayoutBinding};
    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = bindings.size();
    layoutInfo.pBindings = bindings.data();

    if (vk_device.createDescriptorSetLayout(&layoutInfo, nullptr, m_descriptorSetLayout.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.present") << "Failed to create material descriptor set layout." << std::endl;
    }

    //----- Descriptor set

    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_descriptorSetLayout;

    if (vk_device.allocateDescriptorSets(&allocInfo, &m_descriptorSet) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.present") << "Failed to create descriptor set." << std::endl;
    }

    //----- Uniform buffers

    // @todo Have helpers - a Buffer Holder!
    // Create uniform buffers
    vk::DeviceSize cameraBufferSize = sizeof(CameraUbo);
    vk::DeviceSize lightBufferSize = sizeof(LightUbo);

    vk::BufferUsageFlags bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc;
    vk::MemoryPropertyFlags memoryPropertyFlags =
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    vulkan::createBuffer(m_engine.device(), cameraBufferSize, bufferUsageFlags, memoryPropertyFlags, m_cameraUniformStagingBuffer,
                         m_cameraUniformStagingBufferMemory);
    vulkan::createBuffer(m_engine.device(), lightBufferSize, bufferUsageFlags, memoryPropertyFlags, m_lightUniformStagingBuffer,
                         m_lightUniformStagingBufferMemory);

    bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer;
    memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
    vulkan::createBuffer(m_engine.device(), cameraBufferSize, bufferUsageFlags, memoryPropertyFlags, m_cameraUniformBuffer,
                         m_cameraUniformBufferMemory);
    vulkan::createBuffer(m_engine.device(), lightBufferSize, bufferUsageFlags, memoryPropertyFlags, m_lightUniformBuffer,
                         m_lightUniformBufferMemory);

    // Set them up
    vk::DescriptorBufferInfo cameraBufferInfo;
    cameraBufferInfo.buffer = m_cameraUniformBuffer;
    cameraBufferInfo.range = sizeof(CameraUbo);

    vk::DescriptorBufferInfo lightBufferInfo;
    lightBufferInfo.buffer = m_lightUniformBuffer;
    lightBufferInfo.range = sizeof(LightUbo);

    std::array<vk::WriteDescriptorSet, 2> descriptorWrites;

    descriptorWrites[0].dstSet = m_descriptorSet;
    descriptorWrites[0].dstBinding = 0u;
    descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &cameraBufferInfo;

    descriptorWrites[1].dstSet = m_descriptorSet;
    descriptorWrites[1].dstBinding = 1u;
    descriptorWrites[1].descriptorType = vk::DescriptorType::eUniformBuffer;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &lightBufferInfo;

    vk_device.updateDescriptorSets(descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

    logger.log().tab(-1);
}

void Epiphany::update()
{
    logger.log() << "Updating Epiphany stage." << std::endl;
    logger.log().tab(1);

    createRenderPass();
    createGraphicsPipeline();
    createResources();
    createFramebuffers();

    logger.log().tab(-1);
}

void Epiphany::render(const vk::CommandBuffer& commandBuffer, uint32_t /*frameIndex*/)
{
    //----- Prologue

    // Set render pass
    std::array<vk::ClearValue, 1> clearValues;
    // clearValues[0] does not matter

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_framebuffer;
    renderPassInfo.renderArea.setOffset({0, 0});
    renderPassInfo.renderArea.setExtent(m_engine.swapchain().extent());
    renderPassInfo.setClearValueCount(clearValues.size()).setPClearValues(clearValues.data());

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

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

void Epiphany::createRenderPass()
{
    // @cleanup HPP
    auto& device = m_engine.device();
    const auto& vk_device = device.vk();

    // Epiphany attachement
    vk::Format presentAttachmentFormat = vk::Format(m_engine.swapchain().imageFormat()); // @cleanup HPP
    vk::AttachmentReference presentAttachmentRef{0, vk::ImageLayout::eColorAttachmentOptimal};

    vk::AttachmentDescription presentAttachment;
    presentAttachment.setFormat(presentAttachmentFormat);
    presentAttachment.setSamples(vk::SampleCountFlagBits::e1);
    presentAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
    presentAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    presentAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    presentAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    presentAttachment.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

    std::array<vk::AttachmentReference, 1> colorAttachmentsRefs = {presentAttachmentRef};
    std::array<vk::AttachmentDescription, 1> attachments = {presentAttachment};

    // Subpass
    vk::SubpassDescription subpass;
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    subpass.setColorAttachmentCount(colorAttachmentsRefs.size()).setPColorAttachments(colorAttachmentsRefs.data());

    vk::SubpassDependency dependency;
    dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
    dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    dependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    dependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

    // The render pass indeed
    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.setAttachmentCount(attachments.size()).setPAttachments(attachments.data());
    renderPassInfo.setSubpassCount(1).setPSubpasses(&subpass);
    renderPassInfo.setDependencyCount(1).setPDependencies(&dependency);

    if (vk_device.createRenderPass(&renderPassInfo, nullptr, m_renderPass.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.present") << "Failed to create render pass." << std::endl;
    }
}

void Epiphany::createGraphicsPipeline()
{
    // @cleanup HPP Remove this second device, as it will be casted automatically
    auto& device = m_engine.device();
    const auto& vk_device = device.vk();

    // Shader stages
    vk::PipelineShaderStageCreateInfo vertShaderStageInfo;
    vertShaderStageInfo.setStage(vk::ShaderStageFlagBits::eVertex);
    vertShaderStageInfo.setModule(m_vertShaderModule);
    vertShaderStageInfo.setPName("main");

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo;
    fragShaderStageInfo.setStage(vk::ShaderStageFlagBits::eFragment);
    fragShaderStageInfo.setModule(m_fragShaderModule);
    fragShaderStageInfo.setPName("main");

    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Vertex input
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;

    // Input assembly
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
    inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList);

    // Viewport and scissor
    auto& extent = m_engine.swapchain().extent();
    vk::Rect2D scissor{{0, 0}, extent};
    vk::Viewport viewport{0.f, 0.f};
    viewport.setWidth(extent.width).setHeight(extent.height);
    viewport.setMinDepth(0.f).setMaxDepth(1.f);

    vk::PipelineViewportStateCreateInfo viewportState;
    viewportState.setScissorCount(1).setPScissors(&scissor);
    viewportState.setViewportCount(1).setPViewports(&viewport);

    // Rasterizer
    vk::PipelineRasterizationStateCreateInfo rasterizer;
    rasterizer.setLineWidth(1.f);
    rasterizer.setCullMode(vk::CullModeFlagBits::eNone);

    // Multi-sample
    vk::PipelineMultisampleStateCreateInfo multisampling;
    multisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1);
    multisampling.setMinSampleShading(1.f);

    // Depth buffer
    // Not used

    // Color-blending
    vk::PipelineColorBlendAttachmentState presentBlendAttachment;
    presentBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                                             | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

    std::array<vk::PipelineColorBlendAttachmentState, 1> colorBlendAttachments = {presentBlendAttachment};

    vk::PipelineColorBlendStateCreateInfo colorBlending;
    colorBlending.setLogicOp(vk::LogicOp::eCopy);
    colorBlending.setAttachmentCount(colorBlendAttachments.size()).setPAttachments(colorBlendAttachments.data());

    // Dynamic state
    // Not used yet VkDynamicState

    // Pipeline layout
    // __Note__: Order IS important, as sets numbers in shader correspond to order of appearance in this list
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;

    if (vk_device.createPipelineLayout(&pipelineLayoutInfo, nullptr, m_pipelineLayout.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.present") << "Failed to create pipeline layout." << std::endl;
    }

    // Graphics pipeline indeed
    vk::GraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.setStageCount(2).setPStages(shaderStages);
    pipelineInfo.setPVertexInputState(&vertexInputInfo);
    pipelineInfo.setPInputAssemblyState(&inputAssembly);
    pipelineInfo.setPViewportState(&viewportState);
    pipelineInfo.setPRasterizationState(&rasterizer);
    pipelineInfo.setPMultisampleState(&multisampling);
    pipelineInfo.setPColorBlendState(&colorBlending);
    pipelineInfo.setLayout(m_pipelineLayout);
    pipelineInfo.setRenderPass(m_renderPass);

    if (vk_device.createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, m_pipeline.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.present") << "Failed to create graphics pipeline." << std::endl;
    }

    if (vk_device.createPipelineLayout(&pipelineLayoutInfo, nullptr, m_pipelineLayout.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.present") << "Failed to create graphics pipeline layout." << std::endl;
    }
}

void Epiphany::createResources()
{
    auto extent = m_engine.swapchain().extent();

    // Target
    auto format = vk::Format::eB8G8R8A8Unorm;
    // @cleanup HPP
    m_imageHolder.create(format, vk::Extent2D(extent), vk::ImageAspectFlagBits::eColor);
    vk::ImageLayout oldLayout = vk::ImageLayout::ePreinitialized;
    vk::ImageLayout newLayout = vk::ImageLayout::eTransferDstOptimal;
    vulkan::transitionImageLayout(m_engine.device(), m_engine.commandPool(), m_imageHolder.image().castOld(),
                                  reinterpret_cast<VkImageLayout&>(oldLayout), reinterpret_cast<VkImageLayout&>(newLayout));
}

void Epiphany::createFramebuffers()
{
    // @cleanup HPP
    const auto& vk_device = m_engine.device().vk();

    // @fixme We are still presenting something to the screen at this render pass,
    // but it should not be here, and so swapchain info neither.
    auto& swapchain = m_engine.swapchain();

    // Framebuffer
    vk::FramebufferCreateInfo framebufferInfo;
    framebufferInfo.renderPass = m_renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &m_imageHolder.view();
    framebufferInfo.width = swapchain.extent().width;
    framebufferInfo.height = swapchain.extent().height;
    framebufferInfo.layers = 1;

    if (vk_device.createFramebuffer(&framebufferInfo, nullptr, m_framebuffer.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.g-buffer") << "Failed to create framebuffers." << std::endl;
    }
}

// @todo Use helper function (merge with RmMaterial ones)
void Epiphany::normalImageView(const vk::ImageView& imageView, const vk::Sampler& sampler)
{
    // @cleanup HPP
    const auto& vk_device = m_engine.device().vk();

    vk::DescriptorImageInfo imageInfo;
    // @note Correspond to the final layout specified at previous pass
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

    vk_device.updateDescriptorSets(1u, &descriptorWrite, 0, nullptr);
}

void Epiphany::albedoImageView(const vk::ImageView& imageView, const vk::Sampler& sampler)
{
    // @cleanup HPP
    const auto& vk_device = m_engine.device().vk();

    vk::DescriptorImageInfo imageInfo;
    // @note Correspond to the final layout specified at previous pass
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

    vk_device.updateDescriptorSets(1u, &descriptorWrite, 0, nullptr);
}

void Epiphany::updateUbos()
{
    // @cleanup HPP
    const auto& vk_device = m_engine.device().vk();

    //----- Camera UBO

    if (m_engine.cameras().size() > 0) {
        CameraUbo ubo;
        ubo.wPosition = m_engine.camera(0).position();

        void* data;
        vk::MemoryMapFlags memoryMapFlags;
        vk_device.mapMemory(m_cameraUniformStagingBufferMemory, 0, sizeof(CameraUbo), memoryMapFlags, &data);
        memcpy(data, &ubo, sizeof(CameraUbo));
        vk_device.unmapMemory(m_cameraUniformStagingBufferMemory);

        // @cleanup HPP
        vulkan::copyBuffer(m_engine.device(), vk::CommandPool(m_engine.commandPool()), m_cameraUniformStagingBuffer,
                           m_cameraUniformBuffer, sizeof(CameraUbo));
    }

    //----- Light UBO

    if (m_engine.pointLights().size() > 0) {
        CameraUbo ubo;
        ubo.wPosition = m_engine.pointLight(0).position();

        void* data;
        vk::MemoryMapFlags memoryMapFlags;
        vk_device.mapMemory(m_lightUniformStagingBufferMemory, 0, sizeof(LightUbo), memoryMapFlags, &data);
        memcpy(data, &ubo, sizeof(LightUbo));
        vk_device.unmapMemory(m_lightUniformStagingBufferMemory);

        // @cleanup HPP
        vulkan::copyBuffer(m_engine.device(), vk::CommandPool(m_engine.commandPool()), m_lightUniformStagingBuffer,
                           m_lightUniformBuffer, sizeof(LightUbo));
    }
}
