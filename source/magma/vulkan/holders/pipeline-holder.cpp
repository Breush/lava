#include "./pipeline-holder.hpp"

#include "../render-engine-impl.hpp"

using namespace lava::magma::vulkan;
using namespace lava::chamber;

PipelineHolder::PipelineHolder(RenderEngine::Impl& engine)
    : m_engine(engine)
    , m_pipelineLayout{engine.device()}
    , m_pipeline{engine.device()}
{
}

void PipelineHolder::init(uint32_t subpassIndex)
{
    m_subpassIndex = subpassIndex;

    initPipelineLayout();
}

void PipelineHolder::update(vk::Extent2D extent, vk::PolygonMode polygonMode)
{
    // Cannot reconstruct pipeline with an active GPU
    m_engine.device().waitIdle();

    //--- Vertex input

    vk::PipelineVertexInputStateCreateInfo vertexInputState;
    vk::VertexInputBindingDescription vertexInputBindingDescription;
    std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions;

    if (m_vertexInput.attributes.size() > 0u) {
        vertexInputBindingDescription.binding = 0;
        vertexInputBindingDescription.stride = m_vertexInput.stride;
        vertexInputBindingDescription.inputRate = vk::VertexInputRate::eVertex;

        vertexInputAttributeDescriptions.resize(m_vertexInput.attributes.size());
        for (auto i = 0u; i < m_vertexInput.attributes.size(); ++i) {
            const auto& attribute = m_vertexInput.attributes[i];
            vertexInputAttributeDescriptions[i].binding = 0;
            vertexInputAttributeDescriptions[i].location = i;
            vertexInputAttributeDescriptions[i].format = attribute.format;
            vertexInputAttributeDescriptions[i].offset = attribute.offset;
        }

        vertexInputState.vertexBindingDescriptionCount = 1;
        vertexInputState.pVertexBindingDescriptions = &vertexInputBindingDescription;
        vertexInputState.vertexAttributeDescriptionCount = vertexInputAttributeDescriptions.size();
        vertexInputState.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();
    }

    //--- Input assembly

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
    inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;

    //--- Viewport state

    vk::Rect2D scissor;
    scissor.offset = vk::Offset2D{0, 0};
    scissor.extent = extent;

    vk::Viewport viewport;
    viewport.width = extent.width;
    viewport.height = extent.height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    vk::PipelineViewportStateCreateInfo viewportState;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;

    //--- Rasterization state

    vk::PipelineRasterizationStateCreateInfo rasterizationState;
    rasterizationState.lineWidth = 1.f;
    rasterizationState.cullMode = m_cullMode;
    rasterizationState.frontFace = vk::FrontFace::eCounterClockwise;
    rasterizationState.polygonMode = polygonMode;

    //--- Multisample state

    vk::PipelineMultisampleStateCreateInfo multisampleState;
    multisampleState.rasterizationSamples = vk::SampleCountFlagBits::e1;
    multisampleState.minSampleShading = 1.f;

    //--- Depth stencil state

    vk::PipelineDepthStencilStateCreateInfo depthStencilState;

    if (m_depthStencilAttachment) {
        depthStencilState.depthTestEnable = true;
        depthStencilState.depthWriteEnable = m_depthStencilAttachment->depthWriteEnabled;
        depthStencilState.depthCompareOp = vk::CompareOp::eLess;
        depthStencilState.minDepthBounds = 0.f;
        depthStencilState.maxDepthBounds = 1.f;
    }

    //--- Color blend state

    std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachmentStates;
    colorBlendAttachmentStates.resize(m_colorAttachments.size());

    for (auto i = 0u; i < colorBlendAttachmentStates.size(); ++i) {
        auto& colorBlendAttachmentState = colorBlendAttachmentStates[i];
        colorBlendAttachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                                                   | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

        if (m_colorAttachments[i].blending == ColorAttachmentBlending::AlphaBlending) {
            // finalColor.rgb = newAlpha * newColor + (1 - newAlpha) * oldColor;
            // finalColor.a = newAlpha.a;
            colorBlendAttachmentState.blendEnable = true;
            colorBlendAttachmentState.colorBlendOp = vk::BlendOp::eAdd;
            colorBlendAttachmentState.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
            colorBlendAttachmentState.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
            colorBlendAttachmentState.alphaBlendOp = vk::BlendOp::eAdd;
            colorBlendAttachmentState.srcAlphaBlendFactor = vk::BlendFactor::eOne;
            colorBlendAttachmentState.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        }
    }

    vk::PipelineColorBlendStateCreateInfo colorBlendState;
    colorBlendState.attachmentCount = colorBlendAttachmentStates.size();
    colorBlendState.pAttachments = colorBlendAttachmentStates.data();

    //--- Compose pipeline info

    vk::GraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.stageCount = m_shaderStages.size();
    pipelineInfo.pStages = m_shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputState;
    pipelineInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizationState;
    pipelineInfo.pMultisampleState = &multisampleState;
    pipelineInfo.pDepthStencilState = &depthStencilState;
    pipelineInfo.pColorBlendState = &colorBlendState;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = *m_renderPass;
    pipelineInfo.subpass = m_subpassIndex;

    if (m_engine.device().createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, m_pipeline.replace())
        != vk::Result::eSuccess) {
        logger.error("magma.vulkan.stages.render-stage") << "Failed to create graphics pipeline." << std::endl;
    }
}

void PipelineHolder::initPipelineLayout()
{
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = m_descriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = m_descriptorSetLayouts.data();

    if (m_engine.device().createPipelineLayout(&pipelineLayoutInfo, nullptr, m_pipelineLayout.replace())
        != vk::Result::eSuccess) {
        logger.error("magma.vulkan.stages.render-stage") << "Failed to create pipeline layout." << std::endl;
    }
}

void PipelineHolder::add(const vk::DescriptorSetLayout& descriptorSetLayout)
{
    m_descriptorSetLayouts.emplace_back(descriptorSetLayout);
}

void PipelineHolder::add(const vk::PipelineShaderStageCreateInfo& shaderStage)
{
    m_shaderStages.emplace_back(shaderStage);
}

void PipelineHolder::add(const ColorAttachment& colorAttachment)
{
    m_colorAttachments.emplace_back(colorAttachment);
}

void PipelineHolder::set(const DepthStencilAttachment& depthStencilAttachment)
{
    m_depthStencilAttachment = std::make_unique<DepthStencilAttachment>(depthStencilAttachment);
}

void PipelineHolder::add(const InputAttachment& inputAttachment)
{
    m_inputAttachments.emplace_back(inputAttachment);
}
