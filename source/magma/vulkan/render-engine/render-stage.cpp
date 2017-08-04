#include "./render-stage.hpp"

#include <lava/chamber/logger.hpp>

#include "../render-engine-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

RenderStage::RenderStage(RenderEngine::Impl& engine)
    : m_engine(engine)
    , m_renderPass{m_engine.device().vk()}
    , m_pipelineLayout{m_engine.device().vk()}
    , m_pipeline{m_engine.device().vk()}
{
}

//----- IStage

void RenderStage::init()
{
    stageInit();
    initRenderPass();
    initPipelineLayout();
}

void RenderStage::update(const vk::Extent2D& extent)
{
    if (extent.width == 0 || extent.height == 0) {
        logger.warning("magma.vulkan.render-engine.render-stage")
            << "Skipping update with invalid extent " << extent.width << "x" << extent.height << "." << std::endl;
        return;
    }
    else if (m_updateCleverlySkipped && (extent.width == m_extent.width || extent.height == m_extent.height)) {
        logger.info("magma.vulkan.render-engine.render-stage") << "Skipping update with same extent." << std::endl;
        return;
    }

    m_extent = extent;

    stageUpdate();
    updatePipeline();
}

void RenderStage::render(const vk::CommandBuffer& commandBuffer)
{
    stageRender(commandBuffer);
}

//----- Internals API

void RenderStage::add(const vk::DescriptorSetLayout& descriptorSetLayout)
{
    m_descriptorSetLayouts.emplace_back(descriptorSetLayout);
}

void RenderStage::add(const vk::PipelineShaderStageCreateInfo& shaderStage)
{
    m_shaderStages.emplace_back(shaderStage);
}

void RenderStage::add(const ColorAttachment& colorAttachment)
{
    m_colorAttachments.emplace_back(colorAttachment);
}

void RenderStage::set(const DepthStencilAttachment& depthStencilAttachment)
{
    m_depthStencilAttachment = std::make_unique<DepthStencilAttachment>(depthStencilAttachment);
}

//----- Set-up

void RenderStage::initRenderPass()
{
    // @cleanup HPP
    auto& device = m_engine.device();
    const auto& vk_device = device.vk();

    std::vector<vk::AttachmentDescription> attachments;
    std::vector<vk::AttachmentReference> attachmentsReferences;

    // Color attachments
    for (const auto& colorAttachment : m_colorAttachments) {
        attachmentsReferences.emplace_back(attachmentsReferences.size(), vk::ImageLayout::eColorAttachmentOptimal);

        vk::AttachmentDescription attachment;
        attachment.format = colorAttachment.format;
        attachment.samples = vk::SampleCountFlagBits::e1;
        attachment.loadOp = vk::AttachmentLoadOp::eClear;
        attachment.storeOp = vk::AttachmentStoreOp::eStore;
        attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        attachment.finalLayout = colorAttachment.finalLayout;
        attachments.emplace_back(attachment);
    }

    // Depth stencil

    if (m_depthStencilAttachment) {
        attachmentsReferences.emplace_back(attachmentsReferences.size(), vk::ImageLayout::eDepthStencilAttachmentOptimal);

        vk::AttachmentDescription attachment;
        attachment.format = m_depthStencilAttachment->format;
        attachment.samples = vk::SampleCountFlagBits::e1;
        attachment.loadOp = vk::AttachmentLoadOp::eClear;
        attachment.storeOp = vk::AttachmentStoreOp::eStore;
        attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        attachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        attachments.emplace_back(attachment);
    }

    // Subpass
    vk::SubpassDescription subpass;
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = m_colorAttachments.size();
    subpass.pColorAttachments = attachmentsReferences.data();

    if (m_depthStencilAttachment) {
        subpass.pDepthStencilAttachment = attachmentsReferences.data() + m_colorAttachments.size();
    }

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
        logger.error("magma.vulkan.render-engine.render-stage") << "Failed to create render pass." << std::endl;
    }
}

void RenderStage::initPipelineLayout()
{
    // @cleanup HPP
    auto& device = m_engine.device();
    const auto& vk_device = device.vk();

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = m_descriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = m_descriptorSetLayouts.data();

    if (vk_device.createPipelineLayout(&pipelineLayoutInfo, nullptr, m_pipelineLayout.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.render-stage") << "Failed to create pipeline layout." << std::endl;
    }
}

void RenderStage::updatePipeline()
{
    // @cleanup HPP Remove this second device, as it will be casted automatically
    auto& device = m_engine.device();
    const auto& vk_device = device.vk();

    auto vertexInputInfoState = pipelineVertexInputStateCreateInfo();
    auto inputAssemblyState = pipelineInputAssemblyStateCreateInfo();
    auto viewportState = pipelineViewportStateCreateInfo();
    auto rasterizationState = pipelineRasterizationStateCreateInfo();
    auto multisamplingState = pipelineMultisampleStateCreateInfo();
    auto depthStencilState = pipelineDepthStencilStateCreateInfo();
    auto colorBlendState = pipelineColorBlendStateCreateInfo();

    vk::GraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.stageCount = m_shaderStages.size();
    pipelineInfo.pStages = m_shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfoState;
    pipelineInfo.pInputAssemblyState = &inputAssemblyState;
    // @todo TessellationState
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizationState;
    pipelineInfo.pMultisampleState = &multisamplingState;
    pipelineInfo.pDepthStencilState = &depthStencilState;
    pipelineInfo.pColorBlendState = &colorBlendState;
    // @todo DynamicState
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;

    if (vk_device.createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, m_pipeline.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.render-stage") << "Failed to create graphics pipeline." << std::endl;
    }
}

//----- Pipeline creation defaults

vk::PipelineVertexInputStateCreateInfo RenderStage::pipelineVertexInputStateCreateInfo()
{
    vk::PipelineVertexInputStateCreateInfo vertexInputState;

    return vertexInputState;
}

vk::PipelineInputAssemblyStateCreateInfo RenderStage::pipelineInputAssemblyStateCreateInfo()
{
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
    inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;

    return inputAssemblyState;
}

vk::PipelineViewportStateCreateInfo RenderStage::pipelineViewportStateCreateInfo()
{
    m_scissor.offset = vk::Offset2D{0, 0};
    m_scissor.extent = m_extent;

    m_viewport.width = m_extent.width;
    m_viewport.height = m_extent.height;
    m_viewport.minDepth = 0.f;
    m_viewport.maxDepth = 1.f;

    vk::PipelineViewportStateCreateInfo viewportState;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &m_scissor;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &m_viewport;

    return viewportState;
}

vk::PipelineRasterizationStateCreateInfo RenderStage::pipelineRasterizationStateCreateInfo()
{
    vk::PipelineRasterizationStateCreateInfo rasterizationState;
    rasterizationState.lineWidth = 1.f;
    rasterizationState.cullMode = vk::CullModeFlagBits::eNone;

    return rasterizationState;
}

vk::PipelineMultisampleStateCreateInfo RenderStage::pipelineMultisampleStateCreateInfo()
{
    vk::PipelineMultisampleStateCreateInfo multisampleState;
    multisampleState.rasterizationSamples = vk::SampleCountFlagBits::e1;
    multisampleState.minSampleShading = 1.f;

    return multisampleState;
}

vk::PipelineDepthStencilStateCreateInfo RenderStage::pipelineDepthStencilStateCreateInfo()
{
    vk::PipelineDepthStencilStateCreateInfo depthStencilState;

    if (m_depthStencilAttachment) {
        depthStencilState.depthTestEnable = true;
        depthStencilState.depthWriteEnable = true;
        depthStencilState.depthCompareOp = vk::CompareOp::eLess;
        depthStencilState.minDepthBounds = 0.f;
        depthStencilState.maxDepthBounds = 1.f;
    }

    return depthStencilState;
}

vk::PipelineColorBlendStateCreateInfo RenderStage::pipelineColorBlendStateCreateInfo()
{
    m_colorBlendAttachmentStates.resize(m_colorAttachments.size());

    for (auto& colorBlendAttachmentState : m_colorBlendAttachmentStates) {
        colorBlendAttachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                                                   | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    }

    vk::PipelineColorBlendStateCreateInfo colorBlendState;
    colorBlendState.logicOp = vk::LogicOp::eCopy;
    colorBlendState.attachmentCount = m_colorBlendAttachmentStates.size();
    colorBlendState.pAttachments = m_colorBlendAttachmentStates.data();

    return colorBlendState;
}
