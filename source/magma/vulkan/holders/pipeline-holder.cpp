#include "./pipeline-holder.hpp"

#include "../render-engine-impl.hpp"

using namespace lava::magma::vulkan;
using namespace lava::chamber;

PipelineHolder::PipelineHolder(RenderEngine::Impl& engine)
    : m_engine(engine)
{
}

void PipelineHolder::init(PipelineKind kind, uint32_t subpassIndex)
{
    m_kind = kind;
    m_subpassIndex = subpassIndex;

    if (kind == PipelineKind::Graphics) {
        m_pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
    }
    else if (kind == PipelineKind::RayTracing) {
        m_pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eRaygenKHR;
    }

    initPipelineLayout();
}

void PipelineHolder::update(const vk::Extent2D& extent, vk::PolygonMode polygonMode)
{
    // Cannot reconstruct pipeline with an active GPU
    m_engine.device().waitIdle();

    //--- Vertex input

    vk::PipelineVertexInputStateCreateInfo vertexInputState;
    std::vector<vk::VertexInputBindingDescription> vertexInputBindingDescriptions;
    std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions;

    if (!m_vertexInputs.empty()) {
        auto attributeLocation = 0u;
        for (auto binding = 0u; binding < m_vertexInputs.size(); ++binding) {
            const auto& vertexInput = m_vertexInputs[binding];

            auto& vertexInputBindingDescription = vertexInputBindingDescriptions.emplace_back();
            vertexInputBindingDescription.binding = binding;
            vertexInputBindingDescription.stride = vertexInput.stride;
            vertexInputBindingDescription.inputRate = vertexInput.rate;

            for (const auto& attribute : vertexInput.attributes) {
                auto& vertexInputAttributeDescription = vertexInputAttributeDescriptions.emplace_back();
                vertexInputAttributeDescription.binding = binding;
                vertexInputAttributeDescription.location = attributeLocation;
                vertexInputAttributeDescription.format = attribute.format;
                vertexInputAttributeDescription.offset = attribute.offset;
                attributeLocation += 1u;
            }
        }

        vertexInputState.vertexBindingDescriptionCount = vertexInputBindingDescriptions.size();
        vertexInputState.pVertexBindingDescriptions = vertexInputBindingDescriptions.data();
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

    std::vector<vk::DynamicState> dynamicStates;
    if (m_dynamicViewportEnabled) {
        dynamicStates.emplace_back(vk::DynamicState::eViewport);
    }

    vk::PipelineDynamicStateCreateInfo dynamicState;
    dynamicState.dynamicStateCount = dynamicStates.size();
    dynamicState.pDynamicStates = dynamicStates.data();

    //--- Rasterization state

    vk::PipelineRasterizationStateCreateInfo rasterizationState;
    rasterizationState.lineWidth = 1.f;
    rasterizationState.cullMode = m_cullMode;
    rasterizationState.frontFace = vk::FrontFace::eCounterClockwise;
    rasterizationState.polygonMode = polygonMode;

    //--- Multisample state

    vk::PipelineMultisampleStateCreateInfo multisampleState;
    multisampleState.rasterizationSamples = m_sampleCount;
    multisampleState.minSampleShading = 1.f;

    //--- Depth stencil state

    vk::PipelineDepthStencilStateCreateInfo depthStencilState;

    if (m_depthStencilAttachment) {
        depthStencilState.depthTestEnable = true;
        depthStencilState.depthWriteEnable = m_depthStencilAttachment->depthWriteEnabled;
        depthStencilState.depthCompareOp =
            (m_depthStencilAttachment->depthWriteEnabled) ? vk::CompareOp::eLess : vk::CompareOp::eLessOrEqual;
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
            // finalColor.rgb = (new.a) * new.rgb + (1 - new.a) * old.rgb;
            // finalColor.a = new.a + (1 - new.a) * old.a;
            colorBlendAttachmentState.blendEnable = true;
            colorBlendAttachmentState.colorBlendOp = vk::BlendOp::eAdd;
            colorBlendAttachmentState.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
            colorBlendAttachmentState.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
            colorBlendAttachmentState.alphaBlendOp = vk::BlendOp::eAdd;
            colorBlendAttachmentState.srcAlphaBlendFactor = vk::BlendFactor::eOne;
            colorBlendAttachmentState.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        }
    }

    vk::PipelineColorBlendStateCreateInfo colorBlendState;
    colorBlendState.attachmentCount = colorBlendAttachmentStates.size();
    colorBlendState.pAttachments = colorBlendAttachmentStates.data();

    //--- Compose pipeline info

    vk::GraphicsPipelineCreateInfo createInfo;
    createInfo.stageCount = m_shaderStages.size();
    createInfo.pStages = m_shaderStages.data();
    createInfo.pVertexInputState = &vertexInputState;
    createInfo.pInputAssemblyState = &inputAssemblyState;
    createInfo.pViewportState = &viewportState;
    createInfo.pRasterizationState = &rasterizationState;
    createInfo.pMultisampleState = &multisampleState;
    createInfo.pDepthStencilState = &depthStencilState;
    createInfo.pColorBlendState = &colorBlendState;
    createInfo.pDynamicState = &dynamicState;
    createInfo.layout = m_pipelineLayout.get();
    createInfo.renderPass = *m_renderPass;
    createInfo.subpass = m_subpassIndex;

    auto result = m_engine.device().createGraphicsPipelineUnique(nullptr, createInfo);
    m_pipeline = vulkan::checkMove(result, "pipeline-holder", "Unable to create graphics pipeline.");
}

void PipelineHolder::updateRaytracing()
{
    vk::RayTracingPipelineCreateInfoKHR createInfo;
    createInfo.stageCount = m_shaderStages.size();
    createInfo.pStages = m_shaderStages.data();
    createInfo.groupCount = m_shaderGroups.size();
    createInfo.pGroups = m_shaderGroups.data();
    createInfo.maxPipelineRayRecursionDepth = 1;
    createInfo.layout = m_pipelineLayout.get();

    auto result = m_engine.device().createRayTracingPipelineKHRUnique(nullptr, nullptr, createInfo);
    m_pipeline = vulkan::checkMove(result, "pipeline-holder", "Unable to create raytracing pipeline.");
}

void PipelineHolder::initPipelineLayout()
{
    vk::PipelineLayoutCreateInfo createInfo;
    createInfo.setLayoutCount = m_descriptorSetLayouts.size();
    createInfo.pSetLayouts = m_descriptorSetLayouts.data();

    if (m_pushConstantRange.size > 0) {
        createInfo.pushConstantRangeCount = 1;
        createInfo.pPushConstantRanges = &m_pushConstantRange;
    }

    auto result = m_engine.device().createPipelineLayoutUnique(createInfo);
    m_pipelineLayout = vulkan::checkMove(result, "pipeline-holder", "Unable to create pipeline layout.");
}

void PipelineHolder::add(const vk::DescriptorSetLayout& descriptorSetLayout)
{
    m_descriptorSetLayouts.emplace_back(descriptorSetLayout);
}

void PipelineHolder::add(const vk::PipelineShaderStageCreateInfo& shaderStage)
{
    m_shaderStages.emplace_back(shaderStage);
}

void PipelineHolder::add(const vk::RayTracingShaderGroupCreateInfoKHR& shaderGroup)
{
    m_shaderGroups.emplace_back(shaderGroup);
}

void PipelineHolder::add(const ColorAttachment& colorAttachment)
{
    m_colorAttachments.emplace_back(colorAttachment);
}

void PipelineHolder::set(const DepthStencilAttachment& depthStencilAttachment)
{
    m_depthStencilAttachment = std::make_optional<DepthStencilAttachment>(depthStencilAttachment);
}

void PipelineHolder::add(const InputAttachment& inputAttachment)
{
    m_inputAttachments.emplace_back(inputAttachment);
}

void PipelineHolder::set(const ResolveAttachment& resolveAttachment)
{
    m_resolveAttachment = std::make_optional<ResolveAttachment>(resolveAttachment);
}

void PipelineHolder::add(const VertexInput& vertexInput)
{
    m_vertexInputs.emplace_back(vertexInput);
}

void PipelineHolder::addPushConstantRange(uint32_t size)
{
    m_pushConstantRange.size += size;
}
