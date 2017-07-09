#include "./present.hpp"

#include <lava/chamber/logger.hpp>

#include "../buffer.hpp"
#include "../image.hpp"
#include "../render-engine-impl.hpp"
#include "../shader.hpp"
#include "../vertex.hpp"

namespace {
    struct Vertex {
        glm::vec2 position;
    };
}

using namespace lava::magma;
using namespace lava::chamber;

Present::Present(RenderEngine::Impl& engine)
    : m_engine(engine)
    , m_renderPass{m_engine.device().vk()}
    , m_pipelineLayout{m_engine.device().vk()}
    , m_pipeline{m_engine.device().vk()}
    , m_vertexBuffer{m_engine.device().vk()}
    , m_vertexBufferMemory{m_engine.device().vk()}
{
}

// @fixme Still should be a beginRender and endRender,
// as we're going to bind the right image view to the descriptor
// Or should we hold a descriptor? (Probably as that very specific to this step)
void Present::render(const vk::CommandBuffer& commandBuffer, uint32_t frameIndex)
{
    const auto& framebuffer = m_framebuffers[frameIndex];

    // Set render pass
    std::array<vk::ClearValue, 1> clearValues;
    // clearValues[0] does not matter

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.setOffset({0, 0});
    renderPassInfo.renderArea.setExtent(m_engine.swapchain().extent());
    renderPassInfo.setClearValueCount(clearValues.size()).setPClearValues(clearValues.data());

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    // Bind pipeline
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);

    // Add the vertex buffer
    vk::Buffer vertexBuffers[] = {m_vertexBuffer};
    vk::DeviceSize offsets[] = {0};
    commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

    // Draw
    commandBuffer.draw(1, 1, 0, 0);

    // Epilogue
    commandBuffer.endRenderPass();
}

void Present::createRenderPass()
{
    logger.info("magma.vulkan.present") << "Creating render pass." << std::endl;

    // @cleanup HPP
    auto& device = m_engine.device();
    const auto& vk_device = device.vk();

    // Present attachement
    vk::Format presentAttachmentFormat = vk::Format(m_engine.swapchain().imageFormat()); // @cleanup HPP
    vk::AttachmentReference presentAttachmentRef{0, vk::ImageLayout::eColorAttachmentOptimal};

    vk::AttachmentDescription presentAttachment;
    presentAttachment.setFormat(presentAttachmentFormat);
    presentAttachment.setSamples(vk::SampleCountFlagBits::e1);
    presentAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
    presentAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    presentAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    presentAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    presentAttachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR); // @todo This decides what's shown

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
        logger.error("magma.vulkan.present") << "Failed to create render pass." << std::endl;
    }
}

void Present::createGraphicsPipeline()
{
    logger.info("magma.vulkan.present") << "Creating graphics pipeline." << std::endl;

    // @cleanup HPP Remove this second device, as it will be casted automatically
    auto& device = m_engine.device();
    const auto& vk_device = device.vk();

    auto vertShaderCode = vulkan::readGlslShaderFile("./data/shaders/present.vert");
    auto fragShaderCode = vulkan::readGlslShaderFile("./data/shaders/present.frag");

    vulkan::Capsule<VkShaderModule> vertShaderModule{device.capsule(), vkDestroyShaderModule};
    vulkan::Capsule<VkShaderModule> fragShaderModule{device.capsule(), vkDestroyShaderModule};

    vulkan::createShaderModule(device, vertShaderCode, vertShaderModule);
    vulkan::createShaderModule(device, fragShaderCode, fragShaderModule);

    // Shader stages
    vk::PipelineShaderStageCreateInfo vertShaderStageInfo;
    vertShaderStageInfo.setStage(vk::ShaderStageFlagBits::eVertex);
    vertShaderStageInfo.setModule(vk::ShaderModule(vertShaderModule)); // @cleanup HPP
    vertShaderStageInfo.setPName("main");

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo;
    fragShaderStageInfo.setStage(vk::ShaderStageFlagBits::eFragment);
    fragShaderStageInfo.setModule(vk::ShaderModule(fragShaderModule)); // @cleanup HPP
    fragShaderStageInfo.setPName("main");

    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Vertex input
    // @fixme Not that at all
    auto bindingDescription = vulkan::Vertex::bindingDescription();
    auto attributeDescriptions = vulkan::Vertex::attributeDescriptions();

    // @cleanup HPP
    vk::VertexInputBindingDescription vk_bindingDescription(bindingDescription);
    std::vector<vk::VertexInputAttributeDescription> vk_attributeDescriptions;
    for (auto& attribute : attributeDescriptions) {
        vk_attributeDescriptions.emplace_back(attribute);
    }

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.setVertexBindingDescriptionCount(1).setPVertexBindingDescriptions(&vk_bindingDescription);
    vertexInputInfo.setVertexAttributeDescriptionCount(vk_attributeDescriptions.size())
        .setPVertexAttributeDescriptions(vk_attributeDescriptions.data());

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
    std::array<vk::DescriptorSetLayout, 1> setLayouts = {vk::DescriptorSetLayout(m_engine.meshDescriptorSetLayout())};

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    // @todo Have own descriptor set layouts
    pipelineLayoutInfo.setSetLayoutCount(setLayouts.size()).setPSetLayouts(setLayouts.data());

    if (vk_device.createPipelineLayout(&pipelineLayoutInfo, nullptr, m_pipelineLayout.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.present") << "Failed to create pipeline layout." << std::endl;
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
        logger.error("magma.vulkan.present") << "Failed to create graphics pipeline." << std::endl;
    }

    if (vk_device.createPipelineLayout(&pipelineLayoutInfo, nullptr, m_pipelineLayout.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.present") << "Failed to create graphics pipeline layout." << std::endl;
    }
}

void Present::createResources()
{
    logger.info("magma.vulkan.present") << "Creating resources." << std::endl;

    // @cleanup HPP
    auto& device = m_engine.device();
    const auto& vk_device = device.vk();

    //----- Our vertex buffer

    // Describing a simple quad, not indexed
    std::vector<Vertex> vertices = {{{0.f, 0.f}}, {{0.f, 1.f}}, {{1.f, 1.f}}, {{1.f, 1.f}}, {{1.f, 0.f}}, {{0.f, 0.f}}};

    // @todo Would be great to have a BufferHolder to help us set up all that

    vk::DeviceSize bufferSize = sizeof(Vertex) * vertices.size();

    // Staging buffer
    vulkan::Buffer stagingBuffer{vk_device};
    vulkan::DeviceMemory stagingBufferMemory{vk_device};
    vk::BufferUsageFlags bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc;
    vk::MemoryPropertyFlags memoryPropertyFlags =
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    vulkan::createBuffer(device, bufferSize, bufferUsageFlags, memoryPropertyFlags, stagingBuffer, stagingBufferMemory);

    // Store the data to the staging buffer
    void* data;
    vk_device.mapMemory(stagingBufferMemory, 0, bufferSize, {}, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vk_device.unmapMemory(stagingBufferMemory);

    // Actual vertex buffer
    bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
    memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
    vulkan::createBuffer(device, bufferSize, bufferUsageFlags, memoryPropertyFlags, m_vertexBuffer, m_vertexBufferMemory);

    // Copy
    // @cleanup HPP
    vulkan::copyBuffer(device, reinterpret_cast<vk::CommandPool&>(m_engine.commandPool()), stagingBuffer, m_vertexBuffer,
                       bufferSize);
}

void Present::createFramebuffers()
{
    logger.info("magma.vulkan.present") << "Creating frame buffers." << std::endl;

    // @cleanup HPP
    auto& device = m_engine.device();
    const auto& vk_device = device.vk();
    auto& swapchain = m_engine.swapchain();

    m_framebuffers.resize(swapchain.imageViews().size(), vulkan::Framebuffer{m_engine.device().vk()});

    for (size_t i = 0; i < swapchain.imageViews().size(); i++) {
        std::array<vk::ImageView, 1> attachments = {vk::ImageView(swapchain.imageViews()[i])};

        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapchain.extent().width;
        framebufferInfo.height = swapchain.extent().height;
        framebufferInfo.layers = 1;

        if (vk_device.createFramebuffer(&framebufferInfo, nullptr, m_framebuffers[i].replace()) != vk::Result::eSuccess) {
            logger.error("magma.vulkan.present") << "Failed to create framebuffers." << std::endl;
        }
    }
}
