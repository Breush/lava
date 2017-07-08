#include "./g-buffer.hpp"

#include <lava/chamber/logger.hpp>

#include "../render-engine-impl.hpp"
#include "../shader.hpp"
#include "../vertex.hpp"

using namespace lava::magma;
using namespace lava::chamber;

GBuffer::GBuffer(RenderEngine::Impl& engine)
    : m_engine(engine)
    , m_pipelineLayout{m_engine.device().vk(), &vk::Device::destroyPipelineLayout}
    , m_renderPass{m_engine.device().vk(), &vk::Device::destroyRenderPass}
    , m_graphicsPipeline{m_engine.device().vk(), &vk::Device::destroyPipeline}
{
}

void GBuffer::init()
{
    // @todo createRenderPass
    // createGraphicsPipeline();
}

void GBuffer::createGraphicsPipeline()
{
    logger.info("magma.vulkan.g-buffer") << "Creating graphics pipeline." << std::endl;

    // @todo Transform everything to vulkan hpp
    // @todo Remove this second device, as it will be casted automatically
    auto& device = m_engine.device();
    const auto& vk_device = device.vk();

    auto vertShaderCode = vulkan::readGlslShaderFile("./data/shaders/gbuffer.vert");
    auto fragShaderCode = vulkan::readGlslShaderFile("./data/shaders/gbuffer.frag");

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
    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                                           | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

    vk::PipelineColorBlendStateCreateInfo colorBlending;
    colorBlending.setLogicOp(vk::LogicOp::eCopy);
    colorBlending.setAttachmentCount(1).setPAttachments(&colorBlendAttachment);

    // Dynamic state
    // Not used yet VkDynamicState

    // Pipeline layout
    // __Note__: Order IS important, as sets numbers in shader correspond to order of appearance in this list
    // @cleanup vk::DescriptorSetLayout Should be returned directly
    std::array<vk::DescriptorSetLayout, 3> setLayouts = {vk::DescriptorSetLayout(m_engine.cameraDescriptorSetLayout()),
                                                         vk::DescriptorSetLayout(m_engine.materialDescriptorSetLayout()),
                                                         vk::DescriptorSetLayout(m_engine.meshDescriptorSetLayout())};

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setSetLayoutCount(setLayouts.size()).setPSetLayouts(setLayouts.data());

    if (vk_device.createPipelineLayout(&pipelineLayoutInfo, nullptr, m_pipelineLayout.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.pipeline-layout") << "Failed to create pipeline layout." << std::endl;
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

    if (vk_device.createGraphicsPipelines(vk::PipelineCache(), 1, &pipelineInfo, nullptr, m_graphicsPipeline.replace())
        != vk::Result::eSuccess) {
        logger.error("magma.vulkan.graphics-pipeline") << "Failed to create graphics pipeline." << std::endl;
    }

    if (vk_device.createPipelineLayout(&pipelineLayoutInfo, nullptr, m_pipelineLayout.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.graphics-pipeline") << "Failed to create graphics pipeline layout." << std::endl;
    }
}
