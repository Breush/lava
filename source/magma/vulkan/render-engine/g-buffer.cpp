#include "./g-buffer.hpp"

#include <lava/chamber/logger.hpp>

#include "../buffer.hpp"
#include "../image.hpp"
#include "../render-engine-impl.hpp"
#include "../shader.hpp"
#include "../user-data-render.hpp"
#include "../vertex.hpp"

using namespace lava::magma;
using namespace lava::chamber;

GBuffer::GBuffer(RenderEngine::Impl& engine)
    : IStage(engine)
    , m_vertShaderModule{m_engine.device().vk()}
    , m_fragShaderModule{m_engine.device().vk()}
    , m_normalImageHolder{m_engine.device()}
    , m_albedoImageHolder{m_engine.device()}
    , m_ormImageHolder{m_engine.device()}
    , m_depthImageHolder{m_engine.device()}
    , m_framebuffer{m_engine.device().vk()}
{
}

//----- IStage

void GBuffer::init()
{
    logger.log() << "Initializing G-Buffer stage." << std::endl;
    logger.log().tab(1);

    // @cleanup HPP
    auto& device = m_engine.device();
    const auto& vk_device = device.vk();

    //----- Shaders

    auto vertShaderCode = vulkan::readGlslShaderFile("./data/shaders/render-engine/g-buffer.vert");
    auto fragShaderCode = vulkan::readGlslShaderFile("./data/shaders/render-engine/g-buffer.frag");

    vulkan::createShaderModule(vk_device, vertShaderCode, m_vertShaderModule);
    vulkan::createShaderModule(vk_device, fragShaderCode, m_fragShaderModule);

    logger.log().tab(-1);
}

void GBuffer::update()
{
    logger.log() << "Updating G-Buffer stage." << std::endl;
    logger.log().tab(1);

    createRenderPass();
    createGraphicsPipeline();
    createResources();
    createFramebuffers();

    logger.log().tab(-1);
}

void GBuffer::render(const vk::CommandBuffer& commandBuffer, uint32_t /*frameIndex*/)
{
    //----- Prologue

    // Set render pass
    std::array<vk::ClearValue, 4> clearValues;
    clearValues[0].setColor(std::array<float, 4>{0.5f, 0.5f, 0.5f, 1.f});
    clearValues[1].setColor(std::array<float, 4>{0.f, 0.f, 0.f, 1.f});
    clearValues[2].setColor(std::array<float, 4>{1.f, 0.f, 0.f, 1.f});
    clearValues[3].setDepthStencil({1.f, 0u});

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.setRenderPass(m_renderPass);
    renderPassInfo.setFramebuffer(m_framebuffer);
    renderPassInfo.renderArea.setOffset({0, 0});
    renderPassInfo.renderArea.setExtent(m_engine.swapchain().extent());
    renderPassInfo.setClearValueCount(clearValues.size()).setPClearValues(clearValues.data());

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    // Bind pipeline
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);

    //----- Render

    UserDataRenderIn userData;
    userData.commandBuffer = &commandBuffer;
    userData.pipelineLayout = &m_pipelineLayout;

    // Draw all opaque meshes
    for (auto& camera : m_engine.cameras()) {
        camera->render(&userData);
        for (auto& mesh : m_engine.meshes()) {
            mesh->render(&userData);
        }

        // @todo Handle multiple cameras?
        // -> Probably not
        break;
    }

    //----- Epilogue

    commandBuffer.endRenderPass();
}

//----- Internal

void GBuffer::createRenderPass()
{
    // @cleanup HPP
    auto& device = m_engine.device();
    const auto& vk_device = device.vk();

    // Normal attachement
    vk::Format normalAttachmentFormat = vk::Format(m_engine.swapchain().imageFormat()); // @cleanup HPP
    vk::AttachmentReference normalAttachmentRef{0, vk::ImageLayout::eColorAttachmentOptimal};

    vk::AttachmentDescription normalAttachment;
    normalAttachment.setFormat(normalAttachmentFormat);
    normalAttachment.setSamples(vk::SampleCountFlagBits::e1);
    normalAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
    normalAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    normalAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    normalAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    normalAttachment.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

    // Albedo attachement
    vk::Format albedoAttachmentFormat = vk::Format::eB8G8R8A8Unorm;
    vk::AttachmentReference albedoAttachmentRef{1, vk::ImageLayout::eColorAttachmentOptimal};

    vk::AttachmentDescription albedoAttachment;
    albedoAttachment.setFormat(albedoAttachmentFormat);
    albedoAttachment.setSamples(vk::SampleCountFlagBits::e1);
    albedoAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
    albedoAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    albedoAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    albedoAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    albedoAttachment.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

    // ORM attachement
    vk::Format ormAttachmentFormat = vk::Format::eB8G8R8A8Unorm;
    vk::AttachmentReference ormAttachmentRef{2, vk::ImageLayout::eColorAttachmentOptimal};

    vk::AttachmentDescription ormAttachment;
    ormAttachment.setFormat(ormAttachmentFormat);
    ormAttachment.setSamples(vk::SampleCountFlagBits::e1);
    ormAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
    ormAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    ormAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    ormAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    ormAttachment.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

    // Depth attachement
    vk::Format depthAttachmentFormat =
        static_cast<vk::Format>(vulkan::findDepthBufferFormat(device.physicalDevice())); // @cleanup HPP
    vk::AttachmentReference depthAttachmentRef{3, vk::ImageLayout::eDepthStencilAttachmentOptimal};

    vk::AttachmentDescription depthAttachment;
    depthAttachment.setFormat(depthAttachmentFormat);
    depthAttachment.setSamples(vk::SampleCountFlagBits::e1);
    depthAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
    depthAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    depthAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    depthAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    depthAttachment.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    std::array<vk::AttachmentReference, 3> colorAttachmentsRefs = {normalAttachmentRef, albedoAttachmentRef, ormAttachmentRef};
    std::array<vk::AttachmentDescription, 4> attachments = {normalAttachment, albedoAttachment, ormAttachment, depthAttachment};

    // Subpass
    vk::SubpassDescription subpass;
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    subpass.setColorAttachmentCount(colorAttachmentsRefs.size()).setPColorAttachments(colorAttachmentsRefs.data());
    subpass.setPDepthStencilAttachment(&depthAttachmentRef);

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
        logger.error("magma.vulkan.render-engine.g-buffer") << "Failed to create render pass." << std::endl;
    }
}

void GBuffer::createGraphicsPipeline()
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
    vk::Rect2D scissor{{0, 0}, m_engine.swapchain().extent()};
    vk::Viewport viewport{0.f, 0.f};
    // @todo The GBuffer should be configurable, and not take the swapchain to get the extent
    viewport.setWidth(m_engine.swapchain().extent().width).setHeight(m_engine.swapchain().extent().height);
    viewport.setMinDepth(0.f).setMaxDepth(1.f);

    vk::PipelineViewportStateCreateInfo viewportState;
    viewportState.setScissorCount(1).setPScissors(&scissor);
    viewportState.setViewportCount(1).setPViewports(&viewport);

    // Rasterizer
    vk::PipelineRasterizationStateCreateInfo rasterizer;
    rasterizer.setLineWidth(1.f);
    rasterizer.setCullMode(vk::CullModeFlagBits::eBack);
    rasterizer.setFrontFace(vk::FrontFace::eCounterClockwise);

    // Multi-sample
    vk::PipelineMultisampleStateCreateInfo multisampling;
    multisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1);
    multisampling.setMinSampleShading(1.f);

    // Depth buffer
    vk::PipelineDepthStencilStateCreateInfo depthStencil;
    depthStencil.setDepthTestEnable(true);
    depthStencil.setDepthWriteEnable(true);
    depthStencil.setDepthCompareOp(vk::CompareOp::eLess);
    depthStencil.setMinDepthBounds(0.f).setMaxDepthBounds(1.f);

    // Color-blending
    vk::PipelineColorBlendAttachmentState normalBlendAttachment;
    normalBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                                            | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

    vk::PipelineColorBlendAttachmentState albedoBlendAttachment;
    albedoBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                                            | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

    vk::PipelineColorBlendAttachmentState ormBlendAttachment;
    ormBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                                         | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

    std::array<vk::PipelineColorBlendAttachmentState, 3> colorBlendAttachments = {normalBlendAttachment, albedoBlendAttachment,
                                                                                  ormBlendAttachment};

    vk::PipelineColorBlendStateCreateInfo colorBlending;
    colorBlending.setLogicOp(vk::LogicOp::eCopy);
    colorBlending.setAttachmentCount(colorBlendAttachments.size()).setPAttachments(colorBlendAttachments.data());

    // Dynamic state
    // Not used yet VkDynamicState

    // Pipeline layout
    // __Note__: Order IS important, as sets numbers in shader correspond to order of appearance in this list
    // @cleanup HPP vk::DescriptorSetLayout Should be returned directly
    std::array<vk::DescriptorSetLayout, 3> setLayouts = {vk::DescriptorSetLayout(m_engine.cameraDescriptorSetLayout()),
                                                         vk::DescriptorSetLayout(m_engine.materialDescriptorSetLayout()),
                                                         vk::DescriptorSetLayout(m_engine.meshDescriptorSetLayout())};

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setSetLayoutCount(setLayouts.size()).setPSetLayouts(setLayouts.data());

    if (vk_device.createPipelineLayout(&pipelineLayoutInfo, nullptr, m_pipelineLayout.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.g-buffer") << "Failed to create pipeline layout." << std::endl;
    }

    // Graphics pipeline indeed
    vk::GraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.setStageCount(2).setPStages(shaderStages);
    pipelineInfo.setPVertexInputState(&vertexInputInfo);
    pipelineInfo.setPInputAssemblyState(&inputAssembly);
    pipelineInfo.setPViewportState(&viewportState);
    pipelineInfo.setPRasterizationState(&rasterizer);
    pipelineInfo.setPMultisampleState(&multisampling);
    pipelineInfo.setPDepthStencilState(&depthStencil);
    pipelineInfo.setPColorBlendState(&colorBlending);
    pipelineInfo.setLayout(m_pipelineLayout);
    pipelineInfo.setRenderPass(m_renderPass);

    if (vk_device.createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, m_pipeline.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.g-buffer") << "Failed to create graphics pipeline." << std::endl;
    }

    if (vk_device.createPipelineLayout(&pipelineLayoutInfo, nullptr, m_pipelineLayout.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.g-buffer") << "Failed to create graphics pipeline layout." << std::endl;
    }
}

void GBuffer::createResources()
{
    auto extent = m_engine.swapchain().extent();

    // Normal
    auto normalFormat = vk::Format::eB8G8R8A8Unorm;
    // @cleanup HPP
    m_normalImageHolder.create(normalFormat, vk::Extent2D(extent), vk::ImageAspectFlagBits::eColor);
    vk::ImageLayout normalOldLayout = vk::ImageLayout::ePreinitialized;
    vk::ImageLayout normalNewLayout = vk::ImageLayout::eTransferDstOptimal;
    vulkan::transitionImageLayout(m_engine.device(), m_engine.commandPool(), m_normalImageHolder.image().castOld(),
                                  reinterpret_cast<VkImageLayout&>(normalOldLayout),
                                  reinterpret_cast<VkImageLayout&>(normalNewLayout));

    // Albedo
    auto albedoFormat = vk::Format::eB8G8R8A8Unorm;
    // @cleanup HPP
    m_albedoImageHolder.create(albedoFormat, vk::Extent2D(extent), vk::ImageAspectFlagBits::eColor);
    vk::ImageLayout albedoOldLayout = vk::ImageLayout::ePreinitialized;
    vk::ImageLayout albedoNewLayout = vk::ImageLayout::eTransferDstOptimal;
    vulkan::transitionImageLayout(m_engine.device(), m_engine.commandPool(), m_albedoImageHolder.image().castOld(),
                                  reinterpret_cast<VkImageLayout&>(albedoOldLayout),
                                  reinterpret_cast<VkImageLayout&>(albedoNewLayout));

    // ORM
    auto ormFormat = vk::Format::eB8G8R8A8Unorm;
    // @cleanup HPP
    m_ormImageHolder.create(ormFormat, vk::Extent2D(extent), vk::ImageAspectFlagBits::eColor);
    vk::ImageLayout ormOldLayout = vk::ImageLayout::ePreinitialized;
    vk::ImageLayout ormNewLayout = vk::ImageLayout::eTransferDstOptimal;
    vulkan::transitionImageLayout(m_engine.device(), m_engine.commandPool(), m_ormImageHolder.image().castOld(),
                                  reinterpret_cast<VkImageLayout&>(ormOldLayout), reinterpret_cast<VkImageLayout&>(ormNewLayout));

    // Depth
    auto depthFormat = vulkan::findDepthBufferFormat(m_engine.device().physicalDevice());
    // @cleanup HPP
    m_depthImageHolder.create(vk::Format(depthFormat), vk::Extent2D(extent), vk::ImageAspectFlagBits::eDepth);
    vk::ImageLayout depthOldLayout = vk::ImageLayout::eUndefined;
    vk::ImageLayout depthNewLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    vulkan::transitionImageLayout(m_engine.device(), m_engine.commandPool(), m_depthImageHolder.image().castOld(),
                                  reinterpret_cast<VkImageLayout&>(depthOldLayout),
                                  reinterpret_cast<VkImageLayout&>(depthNewLayout));
}

void GBuffer::createFramebuffers()
{
    // @cleanup HPP
    const auto& vk_device = m_engine.device().vk();

    // @fixme We are still presenting something to the screen at this render pass,
    // but it should not be here, and so swapchain info neither.
    auto& swapchain = m_engine.swapchain();

    // Framebuffer
    std::array<vk::ImageView, 4> attachments = {m_normalImageHolder.view(), m_albedoImageHolder.view(), m_ormImageHolder.view(),
                                                m_depthImageHolder.view()};

    vk::FramebufferCreateInfo framebufferInfo;
    framebufferInfo.renderPass = m_renderPass;
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = swapchain.extent().width;
    framebufferInfo.height = swapchain.extent().height;
    framebufferInfo.layers = 1;

    if (vk_device.createFramebuffer(&framebufferInfo, nullptr, m_framebuffer.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.g-buffer") << "Failed to create framebuffers." << std::endl;
    }
}
