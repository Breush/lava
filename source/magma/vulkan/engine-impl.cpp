#include "./engine-impl.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <lava/chamber/logger.hpp>
#include <set>
#include <stb/stb_image.h>
#include <vulkan/vulkan.hpp>

#include "./buffer.hpp"
#include "./image.hpp"
#include "./mesh-impl.hpp"
#include "./proxy.hpp"
#include "./queue.hpp"
#include "./shader.hpp"
#include "./tools.hpp"
#include "./vertex.hpp"

using namespace lava;

Engine::Impl::Impl(Window& window)
    : m_windowHandle(window.getSystemHandle())
    , m_windowExtent({window.videoMode().width, window.videoMode().height})
{
    initVulkan();
}

Engine::Impl::~Impl()
{
    vkDeviceWaitIdle(m_device);
}

void Engine::Impl::draw()
{
    uint32_t imageIndex;
    const auto MAX = std::numeric_limits<uint64_t>::max();
    auto result = vkAcquireNextImageKHR(m_device, m_swapchain, MAX, m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        logger::error("magma.vulkan.draw") << "Failed to acquire swapchain image." << std::endl;
        exit(1);
    }

    // Submit to the queue
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_device.graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        logger::error("magma.vulkan.layer") << "Failed to submit draw command buffer." << std::endl;
        exit(1);
    }

    // Submitting the image back to the swap chain
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(m_device.presentQueue(), &presentInfo);
}

void Engine::Impl::update()
{
    // @todo Get time!
    static float time = 0.f;
    const float dt = 0.001f;
    const auto viewExtent = m_swapchain.extent();

    time += dt;

    // This is basically our camera
    UniformBufferObject ubo = {};
    ubo.model = glm::rotate(glm::mat4(), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.projection = glm::perspective(glm::radians(45.0f), viewExtent.width / (float)viewExtent.height, 0.1f, 10.0f);
    ubo.projection[1][1] *= -1; // Well, this is not OpenGL!

    void* data;
    vkMapMemory(m_device, m_uniformStagingBufferMemory, 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(m_device, m_uniformStagingBufferMemory);

    vulkan::copyBuffer(m_device, m_commandPool, m_uniformStagingBuffer, m_uniformBuffer, sizeof(ubo));

    // Other meshes
    /* for (size_t i = 0; i < m_meshes.size(); ++i) {
        m_meshes[i]->update();
    }*/
}

void Engine::Impl::mode(const VideoMode& mode)
{
    m_windowExtent = {mode.width, mode.height};
    recreateSwapchain();
}

void Engine::Impl::add(Mesh::Impl& mesh)
{
    m_meshes.emplace_back(&mesh);
}

void Engine::Impl::createRenderPass()
{
    // Color attachement
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = m_swapchain.imageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth attachement
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = vulkan::findDepthBufferFormat(m_device.physicalDevice());
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // The render pass indeed
    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, m_renderPass.replace()) != VK_SUCCESS) {
        logger::error("magma.vulkan.render-pass") << "Failed to create render pass." << std::endl;
        exit(1);
    }
}

void Engine::Impl::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, m_descriptorSetLayout.replace()) != VK_SUCCESS) {
        logger::error("magma.vulkan.descriptor-set-layout") << "Failed to create descriptor set layout." << std::endl;
        exit(1);
    }
}

void Engine::Impl::createGraphicsPipeline()
{
    auto vertShaderCode = vulkan::readShaderFile("./data/shaders/triangle.vert.spv");
    auto fragShaderCode = vulkan::readShaderFile("./data/shaders/triangle.frag.spv");

    vulkan::Capsule<VkShaderModule> vertShaderModule{m_device.capsule(), vkDestroyShaderModule};
    vulkan::Capsule<VkShaderModule> fragShaderModule{m_device.capsule(), vkDestroyShaderModule};

    vulkan::createShaderModule(m_device, vertShaderCode, vertShaderModule);
    vulkan::createShaderModule(m_device, fragShaderCode, fragShaderModule);

    // Shader stages
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Vertex input
    auto bindingDescription = vulkan::Vertex::bindingDescription();
    auto attributeDescriptions = vulkan::Vertex::attributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewport and scissor
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapchain.extent().width);
    viewport.height = static_cast<float>(m_swapchain.extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = m_swapchain.extent();

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    // Multi-sample
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    // Depth/stencil
    // @todo Not used yet VkPipelineDepthStencilStateCreateInfo

    // Color-blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // Depth buffer
    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    // Dynamic state
    // @todo Not used yet VkDynamicState

    // Pipeline layout
    VkDescriptorSetLayout setLayouts[] = {m_descriptorSetLayout};
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = setLayouts;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = 0;

    if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, m_pipelineLayout.replace()) != VK_SUCCESS) {
        logger::error("magma.vulkan.pipeline-layout") << "Failed to create pipeline layout." << std::endl;
        exit(1);
    }

    // Graphics pipeline indeed
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, m_graphicsPipeline.replace())
        != VK_SUCCESS) {
        if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, m_pipelineLayout.replace()) != VK_SUCCESS) {
            logger::error("magma.vulkan.graphics-pipeline") << "Failed to create graphics pipeline." << std::endl;
            exit(1);
        }
    }
}

void Engine::Impl::createFramebuffers()
{
    m_swapchainFramebuffers.resize(m_swapchain.imageViews().size(),
                                   vulkan::Capsule<VkFramebuffer>{m_device.capsule(), vkDestroyFramebuffer});

    for (size_t i = 0; i < m_swapchain.imageViews().size(); i++) {
        std::array<VkImageView, 2> attachments = {m_swapchain.imageViews()[i], m_depthImageView};

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_swapchain.extent().width;
        framebufferInfo.height = m_swapchain.extent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, m_swapchainFramebuffers[i].replace()) != VK_SUCCESS) {
            logger::error("magma.vulkan.framebuffer") << "Failed to create framebuffers." << std::endl;
            exit(1);
        }
    }
}

void Engine::Impl::createCommandPool()
{
    auto queueFamilyIndices = vulkan::findQueueFamilies(m_device.physicalDevice(), m_surface);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphics;
    poolInfo.flags = 0;

    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, m_commandPool.replace()) != VK_SUCCESS) {
        logger::error("magma.vulkan.command-pool") << "Failed to create command pool." << std::endl;
        exit(1);
    }
}

void Engine::Impl::createTextureImage()
{
    auto filename = "./data/images/debug.png";
    int texWidth, texHeight, texChannels;
    auto pixels = stbi_load(filename, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        logger::error("magma.image") << "Failed to load image from filename " << filename << std::endl;
        exit(1);
    }

    vulkan::Capsule<VkImage> stagingImage{m_device.capsule(), vkDestroyImage};
    vulkan::Capsule<VkDeviceMemory> stagingImageMemory{m_device.capsule(), vkFreeMemory};
    vulkan::Capsule<VkImageView> stagingImageView{m_device.capsule(), vkDestroyImageView};

    vulkan::createImage(
        m_device, texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingImage, stagingImageMemory);

    VkImageSubresource subresource = {};
    subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource.mipLevel = 0;
    subresource.arrayLayer = 0;

    VkSubresourceLayout stagingImageLayout;
    vkGetImageSubresourceLayout(m_device, stagingImage, &subresource, &stagingImageLayout);
}

void Engine::Impl::createDepthResources()
{
    auto format = vulkan::findDepthBufferFormat(m_device.physicalDevice());
    auto extent = m_swapchain.extent();

    vulkan::createImage(m_device, extent.width, extent.height, format, VK_IMAGE_TILING_OPTIMAL,
                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage,
                        m_depthImageMemory);
    vulkan::createImageView(m_device, m_depthImage, format, VK_IMAGE_ASPECT_DEPTH_BIT, m_depthImageView);

    vulkan::transitionImageLayout(m_device, m_commandPool, m_depthImage, VK_IMAGE_LAYOUT_UNDEFINED,
                                  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void Engine::Impl::createUniformBuffer()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    int bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    int memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vulkan::createBuffer(m_device, bufferSize, bufferUsageFlags, memoryPropertyFlags, m_uniformStagingBuffer,
                         m_uniformStagingBufferMemory);

    bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    vulkan::createBuffer(m_device, bufferSize, bufferUsageFlags, memoryPropertyFlags, m_uniformBuffer, m_uniformBufferMemory);
}

void Engine::Impl::createDescriptorPool()
{
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;
    poolInfo.flags = 0;

    if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, m_descriptorPool.replace()) != VK_SUCCESS) {
        logger::error("magma.vulkan.descriptor-pool") << "Failed to create descriptor pool." << std::endl;
        exit(1);
    }
}

void Engine::Impl::createDescriptorSet()
{
    VkDescriptorSetLayout layouts[] = {m_descriptorSetLayout};
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts;

    if (vkAllocateDescriptorSets(m_device, &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
        logger::error("magma.vulkan.descriptor-sets") << "Failed to create descriptor sets." << std::endl;
        exit(1);
    }

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = m_uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.pImageInfo = nullptr;
    descriptorWrite.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(m_device, 1, &descriptorWrite, 0, nullptr);
}

void Engine::Impl::createCommandBuffers()
{
    // Free previous command buffers if any
    if (m_commandBuffers.size() > 0) {
        vkFreeCommandBuffers(m_device, m_commandPool, m_commandBuffers.size(), m_commandBuffers.data());
    }

    m_commandBuffers.resize(m_swapchainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    if (vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
        logger::error("magma.vulkan.command-buffers") << "Failed to create command buffers." << std::endl;
        exit(1);
    }

    // Start recording
    for (size_t i = 0; i < m_commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = nullptr;

        vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo);

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass;
        renderPassInfo.framebuffer = m_swapchainFramebuffers[i];

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapchain.extent();

        std::array<VkClearValue, 2> clearValues = {};
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

        // Add uniform buffers
        vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSet, 0,
                                nullptr);

        // Other meshes
        for (size_t j = 0; j < m_meshes.size(); ++j) {
            m_meshes[j]->addCommands(m_commandBuffers[i]);
        }

        vkCmdEndRenderPass(m_commandBuffers[i]);

        if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS) {
            logger::error("magma.vulkan.command-buffer") << "Failed to record command buffer." << std::endl;
            exit(1);
        }
    }
}

void Engine::Impl::createSemaphores()
{
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, m_imageAvailableSemaphore.replace()) != VK_SUCCESS
        || vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, m_renderFinishedSemaphore.replace()) != VK_SUCCESS) {

        logger::error("magma.vulkan.command-buffer") << "Failed to create semaphores." << std::endl;
        exit(1);
    }
}

void Engine::Impl::recreateSwapchain()
{
    logger::info("magma.vulkan.swapchain") << "Recreating swapchain." << std::endl;

    vkDeviceWaitIdle(m_device);

    m_swapchain.init(m_surface, m_windowExtent);

    createRenderPass();
    createGraphicsPipeline();
    createDepthResources();
    createFramebuffers();
    createCommandBuffers();
}

void Engine::Impl::initVulkan()
{
    m_instance.init(true);
    m_surface.init(m_windowHandle);
    m_device.init(m_instance.capsule(), m_surface);
    m_swapchain.init(m_surface, m_windowExtent);

    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createCommandPool();
    createTextureImage();
    createDepthResources();
    createFramebuffers();
    createUniformBuffer();
    createDescriptorPool();
    createDescriptorSet();
    createCommandBuffers();
    createSemaphores();
}
