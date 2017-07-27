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
    , m_normalImageHolder{m_engine.device(), m_engine.commandPool()}
    , m_albedoImageHolder{m_engine.device(), m_engine.commandPool()}
    , m_ormImageHolder{m_engine.device(), m_engine.commandPool()}
    , m_depthImageHolder{m_engine.device(), m_engine.commandPool()}
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

void GBuffer::update(const vk::Extent2D& extent)
{
    logger.log() << "Updating G-Buffer stage." << std::endl;
    logger.log().tab(1);

    m_extent = extent;

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
    clearValues[0].color = std::array<float, 4>{0.5f, 0.5f, 0.5f, 1.f};
    clearValues[1].color = std::array<float, 4>{0.2f, 0.6f, 0.4f, 1.f}; // @todo Allow to be configure
    clearValues[2].color = std::array<float, 4>{1.f, 0.f, 0.f, 1.f};
    clearValues[3].depthStencil = vk::ClearDepthStencilValue{1.f, 0u};

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_framebuffer;
    renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderPassInfo.renderArea.extent = m_extent;
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

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
    vk::Format normalAttachmentFormat = vk::Format::eB8G8R8A8Unorm;
    vk::AttachmentReference normalAttachmentRef{0, vk::ImageLayout::eColorAttachmentOptimal};

    vk::AttachmentDescription normalAttachment;
    normalAttachment.format = normalAttachmentFormat;
    normalAttachment.samples = vk::SampleCountFlagBits::e1;
    normalAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    normalAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    normalAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    normalAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    normalAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    // Albedo attachement
    vk::Format albedoAttachmentFormat = vk::Format::eB8G8R8A8Unorm;
    vk::AttachmentReference albedoAttachmentRef{1, vk::ImageLayout::eColorAttachmentOptimal};

    vk::AttachmentDescription albedoAttachment;
    albedoAttachment.format = albedoAttachmentFormat;
    albedoAttachment.samples = vk::SampleCountFlagBits::e1;
    albedoAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    albedoAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    albedoAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    albedoAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    albedoAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    // ORM attachement
    vk::Format ormAttachmentFormat = vk::Format::eB8G8R8A8Unorm;
    vk::AttachmentReference ormAttachmentRef{2, vk::ImageLayout::eColorAttachmentOptimal};

    vk::AttachmentDescription ormAttachment;
    ormAttachment.format = ormAttachmentFormat;
    ormAttachment.samples = vk::SampleCountFlagBits::e1;
    ormAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    ormAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    ormAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    ormAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    ormAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    // Depth attachement
    vk::Format depthAttachmentFormat =
        static_cast<vk::Format>(vulkan::findDepthBufferFormat(device.physicalDevice())); // @cleanup HPP
    vk::AttachmentReference depthAttachmentRef{3, vk::ImageLayout::eDepthStencilAttachmentOptimal};

    vk::AttachmentDescription depthAttachment;
    depthAttachment.format = depthAttachmentFormat;
    depthAttachment.samples = vk::SampleCountFlagBits::e1;
    depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    std::array<vk::AttachmentReference, 3> colorAttachmentsRefs = {normalAttachmentRef, albedoAttachmentRef, ormAttachmentRef};
    std::array<vk::AttachmentDescription, 4> attachments = {normalAttachment, albedoAttachment, ormAttachment, depthAttachment};

    // Subpass
    vk::SubpassDescription subpass;
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = colorAttachmentsRefs.size();
    subpass.pColorAttachments = colorAttachmentsRefs.data();
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    vk::SubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

    // The render pass indeed
    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

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
    vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertShaderStageInfo.module = m_vertShaderModule;
    vertShaderStageInfo.pName = "main";

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo;
    fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragShaderStageInfo.module = m_fragShaderModule;
    fragShaderStageInfo.pName = "main";

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
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &vk_bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = vk_attributeDescriptions.size();
    vertexInputInfo.pVertexAttributeDescriptions = vk_attributeDescriptions.data();

    // Input assembly
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;

    // Viewport and scissor
    vk::Rect2D scissor{{0, 0}, m_extent};
    vk::Viewport viewport{0.f, 0.f};
    viewport.width = m_extent.width;
    viewport.height = m_extent.height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    vk::PipelineViewportStateCreateInfo viewportState;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;

    // Rasterizer
    vk::PipelineRasterizationStateCreateInfo rasterizer;
    rasterizer.lineWidth = 1.f;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eCounterClockwise;

    // Multi-sample
    vk::PipelineMultisampleStateCreateInfo multisampling;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
    multisampling.minSampleShading = 1.f;

    // Depth buffer
    vk::PipelineDepthStencilStateCreateInfo depthStencil;
    depthStencil.depthTestEnable = true;
    depthStencil.depthWriteEnable = true;
    depthStencil.depthCompareOp = vk::CompareOp::eLess;
    depthStencil.minDepthBounds = 0.f;
    depthStencil.maxDepthBounds = 1.f;

    // Color-blending
    vk::PipelineColorBlendAttachmentState normalBlendAttachment;
    normalBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                                           | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    vk::PipelineColorBlendAttachmentState albedoBlendAttachment;
    albedoBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                                           | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    vk::PipelineColorBlendAttachmentState ormBlendAttachment;
    ormBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                                        | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    std::array<vk::PipelineColorBlendAttachmentState, 3> colorBlendAttachments = {normalBlendAttachment, albedoBlendAttachment,
                                                                                  ormBlendAttachment};

    vk::PipelineColorBlendStateCreateInfo colorBlending;
    colorBlending.logicOp = vk::LogicOp::eCopy;
    colorBlending.attachmentCount = colorBlendAttachments.size();
    colorBlending.pAttachments = colorBlendAttachments.data();

    // Dynamic state
    // Not used yet VkDynamicState

    // Pipeline layout
    // __Note__: Order IS important, as sets numbers in shader correspond to order of appearance in this list
    // @cleanup HPP vk::DescriptorSetLayout Should be returned directly
    std::array<vk::DescriptorSetLayout, 3> setLayouts = {vk::DescriptorSetLayout(m_engine.cameraDescriptorSetLayout()),
                                                         vk::DescriptorSetLayout(m_engine.materialDescriptorSetLayout()),
                                                         vk::DescriptorSetLayout(m_engine.meshDescriptorSetLayout())};

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = setLayouts.size();
    pipelineLayoutInfo.pSetLayouts = setLayouts.data();

    if (vk_device.createPipelineLayout(&pipelineLayoutInfo, nullptr, m_pipelineLayout.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.g-buffer") << "Failed to create pipeline layout." << std::endl;
    }

    // Graphics pipeline indeed
    vk::GraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;

    if (vk_device.createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, m_pipeline.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.g-buffer") << "Failed to create graphics pipeline." << std::endl;
    }

    if (vk_device.createPipelineLayout(&pipelineLayoutInfo, nullptr, m_pipelineLayout.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.g-buffer") << "Failed to create graphics pipeline layout." << std::endl;
    }
}

void GBuffer::createResources()
{
    // Normal
    auto normalFormat = vk::Format::eB8G8R8A8Unorm;
    m_normalImageHolder.create(normalFormat, m_extent, vk::ImageAspectFlagBits::eColor);

    // Albedo
    auto albedoFormat = vk::Format::eB8G8R8A8Unorm;
    m_albedoImageHolder.create(albedoFormat, m_extent, vk::ImageAspectFlagBits::eColor);

    // ORM
    auto ormFormat = vk::Format::eB8G8R8A8Unorm;
    m_ormImageHolder.create(ormFormat, m_extent, vk::ImageAspectFlagBits::eColor);

    // Depth
    // @cleanup HPP
    auto depthFormat = vulkan::findDepthBufferFormat(m_engine.device().physicalDevice());
    m_depthImageHolder.create(vk::Format(depthFormat), m_extent, vk::ImageAspectFlagBits::eDepth);
}

void GBuffer::createFramebuffers()
{
    // @cleanup HPP
    const auto& vk_device = m_engine.device().vk();

    // Framebuffer
    std::array<vk::ImageView, 4> attachments = {m_normalImageHolder.view(), m_albedoImageHolder.view(), m_ormImageHolder.view(),
                                                m_depthImageHolder.view()};

    vk::FramebufferCreateInfo framebufferInfo;
    framebufferInfo.renderPass = m_renderPass;
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = m_extent.width;
    framebufferInfo.height = m_extent.height;
    framebufferInfo.layers = 1;

    if (vk_device.createFramebuffer(&framebufferInfo, nullptr, m_framebuffer.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.g-buffer") << "Failed to create framebuffers." << std::endl;
    }
}
