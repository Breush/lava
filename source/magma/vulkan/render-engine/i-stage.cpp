#include "./i-stage.hpp"

#include <lava/chamber/logger.hpp>

#include "../render-engine-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

IStage::IStage(RenderEngine::Impl& engine)
    : m_engine(engine)
    , m_vertexShaderModule{m_engine.device().vk()}
    , m_fragmentShaderModule{m_engine.device().vk()}
    , m_renderPass{m_engine.device().vk()}
    , m_pipelineLayout{m_engine.device().vk()}
    , m_pipeline{m_engine.device().vk()}
{
}

void IStage::update(const vk::Extent2D& extent)
{
    if (extent.width == 0 || extent.height == 0) {
        logger.warning("magma.vulkan.render-engine.i-stage")
            << "Skipping update with invalid extent " << extent.width << "x" << extent.height << "." << std::endl;
        return;
    }
    /* @todo
    else if (extent.width == m_extent.width || extent.height == m_extent.height) {
        logger.warning("magma.vulkan.render-engine.i-stage") << "Skipping update with same extent." << std::endl;
        return;
    }
    */

    m_extent = extent;
}

//----- Pipeline creation defaults

void IStage::updatePipeline()
{
    // @cleanup HPP Remove this second device, as it will be casted automatically
    auto& device = m_engine.device();
    const auto& vk_device = device.vk();

    auto shaderStages = pipelineShaderStageCreateInfos();
    auto vertexInputInfoState = pipelineVertexInputStateCreateInfo();
    auto inputAssemblyState = pipelineInputAssemblyStateCreateInfo();
    auto viewportState = pipelineViewportStateCreateInfo();
    auto rasterizationState = pipelineRasterizationStateCreateInfo();
    auto multisamplingState = pipelineMultisampleStateCreateInfo();
    auto colorBlendState = pipelineColorBlendStateCreateInfo();

    vk::GraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.stageCount = shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfoState;
    pipelineInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizationState;
    pipelineInfo.pMultisampleState = &multisamplingState;
    pipelineInfo.pColorBlendState = &colorBlendState;
    // @todo Missing depth/stencil!
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;

    if (vk_device.createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, m_pipeline.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.i-stage") << "Failed to create graphics pipeline." << std::endl;
    }
}

std::vector<vk::PipelineShaderStageCreateInfo> IStage::pipelineShaderStageCreateInfos()
{
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

    // @todo Have a way to register those via an interface?
    shaderStages.emplace_back(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, m_vertexShaderModule,
                              "main");
    shaderStages.emplace_back(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, m_fragmentShaderModule,
                              "main");

    return shaderStages;
}

vk::PipelineVertexInputStateCreateInfo IStage::pipelineVertexInputStateCreateInfo()
{
    vk::PipelineVertexInputStateCreateInfo vertexInputState;

    return vertexInputState;
}

vk::PipelineInputAssemblyStateCreateInfo IStage::pipelineInputAssemblyStateCreateInfo()
{
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
    inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;

    return inputAssemblyState;
}

vk::PipelineViewportStateCreateInfo IStage::pipelineViewportStateCreateInfo()
{
    // @todo Allow these to be set up from an interface

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

vk::PipelineRasterizationStateCreateInfo IStage::pipelineRasterizationStateCreateInfo()
{
    vk::PipelineRasterizationStateCreateInfo rasterizationState;
    rasterizationState.lineWidth = 1.f;
    rasterizationState.cullMode = vk::CullModeFlagBits::eNone;

    return rasterizationState;
}

vk::PipelineMultisampleStateCreateInfo IStage::pipelineMultisampleStateCreateInfo()
{
    vk::PipelineMultisampleStateCreateInfo multisampleState;
    multisampleState.rasterizationSamples = vk::SampleCountFlagBits::e1;
    multisampleState.minSampleShading = 1.f;

    return multisampleState;
}

vk::PipelineColorBlendStateCreateInfo IStage::pipelineColorBlendStateCreateInfo()
{
    // @todo Allow to set up from an interface, and do not force anything here

    m_colorBlendAttachmentStates.resize(1);

    m_colorBlendAttachmentStates[0].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                                                     | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    vk::PipelineColorBlendStateCreateInfo colorBlendState;
    colorBlendState.logicOp = vk::LogicOp::eCopy;
    colorBlendState.attachmentCount = m_colorBlendAttachmentStates.size();
    colorBlendState.pAttachments = m_colorBlendAttachmentStates.data();

    return colorBlendState;
}
