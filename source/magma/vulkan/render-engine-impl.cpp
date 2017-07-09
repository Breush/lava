#include "./render-engine-impl.hpp"

#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <glslang/Public/ShaderLang.h>
#include <lava/chamber/logger.hpp>
#include <lava/magma/interfaces/render-target.hpp>
#include <set>
#include <stb/stb_image.h>
#include <vulkan/vulkan.hpp>

#include "./buffer.hpp"
#include "./image.hpp"
#include "./meshes/mesh-impl.hpp"
#include "./proxy.hpp"
#include "./queue.hpp"
#include "./shader.hpp"
#include "./tools.hpp"
#include "./user-data-render.hpp"
#include "./vertex.hpp"

using namespace lava::magma;
using namespace lava::chamber;

RenderEngine::Impl::Impl()
{
}

RenderEngine::Impl::~Impl()
{
    vkDeviceWaitIdle(m_device);
}

void RenderEngine::Impl::add(IRenderTarget& renderTarget)
{
    m_renderTargets.emplace_back(&renderTarget);
}

void RenderEngine::Impl::draw()
{
    m_renderTargets[0]->draw();

    uint32_t index;
    const auto MAX = std::numeric_limits<uint64_t>::max();
    auto result = vkAcquireNextImageKHR(m_device, m_swapchain, MAX, m_imageAvailableSemaphore, VK_NULL_HANDLE, &index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        logger.error("magma.vulkan.draw") << "Failed to acquire swapchain image." << std::endl;
    }

    // Record command buffer each frame
    vkDeviceWaitIdle(m_device); // @todo Better wait for a fence on the queue
    auto& commandBuffer = recordCommandBuffer(index);

    // Submit it to the queue
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_device.graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        logger.error("magma.vulkan.layer") << "Failed to submit draw command buffer." << std::endl;
    }

    // Submitting the image back to the swap chain
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &index;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(m_device.presentQueue(), &presentInfo);
}

void RenderEngine::Impl::update()
{
}

void RenderEngine::Impl::createDescriptorSetLayouts()
{
    logger.info("magma.vulkan.render-engine") << "Creating descriptor set layouts." << std::endl;

    {
        // Camera UBO
        VkDescriptorSetLayoutBinding transformsLayoutBinding = {};
        transformsLayoutBinding.binding = 0;
        transformsLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        transformsLayoutBinding.descriptorCount = 1;
        transformsLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        transformsLayoutBinding.pImmutableSamplers = nullptr;

        std::array<VkDescriptorSetLayoutBinding, 1> bindings = {transformsLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, m_cameraDescriptorSetLayout.replace()) != VK_SUCCESS) {
            logger.error("magma.vulkan.descriptor-set-layout") << "Failed to create descriptor set layout." << std::endl;
        }
    }

    // @todo Move ?
    {
        // Model UBO
        VkDescriptorSetLayoutBinding modelLayoutBinding = {};
        modelLayoutBinding.binding = 0;
        modelLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        modelLayoutBinding.descriptorCount = 1;
        modelLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        modelLayoutBinding.pImmutableSamplers = nullptr;

        std::array<VkDescriptorSetLayoutBinding, 1> bindings = {modelLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, m_meshDescriptorSetLayout.replace()) != VK_SUCCESS) {
            logger.error("magma.vulkan.descriptor-set-layout") << "Failed to create mesh descriptor set layout." << std::endl;
        }
    }

    // @todo Move, this is very individual shader-related
    // @note Not so much, as this is what's REQUIRED by G-Buffer
    {
        // Attributes UBO
        VkDescriptorSetLayoutBinding attributesLayoutBinding = {};
        attributesLayoutBinding.binding = 0;
        attributesLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        attributesLayoutBinding.descriptorCount = 1;
        attributesLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        attributesLayoutBinding.pImmutableSamplers = nullptr;

        // Sampler
        VkDescriptorSetLayoutBinding normalMapLayoutBinding = {};
        normalMapLayoutBinding.binding = 1;
        normalMapLayoutBinding.descriptorCount = 1;
        normalMapLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        normalMapLayoutBinding.pImmutableSamplers = nullptr;
        normalMapLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        // Sampler
        VkDescriptorSetLayoutBinding baseColorLayoutBinding = {};
        baseColorLayoutBinding.binding = 2;
        baseColorLayoutBinding.descriptorCount = 1;
        baseColorLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        baseColorLayoutBinding.pImmutableSamplers = nullptr;
        baseColorLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        // Sampler
        VkDescriptorSetLayoutBinding ormLayoutBinding = {};
        ormLayoutBinding.binding = 3;
        ormLayoutBinding.descriptorCount = 1;
        ormLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        ormLayoutBinding.pImmutableSamplers = nullptr;
        ormLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 4> bindings = {attributesLayoutBinding, baseColorLayoutBinding,
                                                                normalMapLayoutBinding, ormLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, m_materialDescriptorSetLayout.replace()) != VK_SUCCESS) {
            logger.error("magma.vulkan.descriptor-set-layout") << "Failed to create material descriptor set layout." << std::endl;
        }
    }
}

void RenderEngine::Impl::createPipelines()
{
    logger.info("magma.vulkan.render-engine") << "Creating render pipelines." << std::endl;

    // Render passes
    m_gBuffer.createRenderPass();

    // Pipelines
    m_gBuffer.createGraphicsPipeline();

    // Resources
    m_gBuffer.createResources();

    // Framebuffers
    m_gBuffer.createFramebuffers();
}

void RenderEngine::Impl::createCommandPool()
{
    logger.info("magma.vulkan.render-engine") << "Creating command pool." << std::endl;

    auto queueFamilyIndices = vulkan::findQueueFamilies(m_device.physicalDevice(), m_surface);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphics;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, m_commandPool.replace()) != VK_SUCCESS) {
        logger.error("magma.vulkan.command-pool") << "Failed to create command pool." << std::endl;
    }
}

void RenderEngine::Impl::createDummyTexture()
{
    logger.info("magma.vulkan.render-engine") << "Creating dummy texture." << std::endl;

    // @todo This should be refactored with what's in RmMaterial

    magma::vulkan::Capsule<VkImage> stagingImage{m_device.capsule(), vkDestroyImage};
    magma::vulkan::Capsule<VkDeviceMemory> stagingImageMemory{m_device.capsule(), vkFreeMemory};
    magma::vulkan::Capsule<VkImageView> stagingImageView{m_device.capsule(), vkDestroyImageView};

    magma::vulkan::createImage(
        m_device, 1u, 1u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingImage, stagingImageMemory);

    VkImageSubresource subresource = {};
    subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource.mipLevel = 0;
    subresource.arrayLayer = 0;

    VkSubresourceLayout stagingImageLayout;
    vkGetImageSubresourceLayout(m_device, stagingImage, &subresource, &stagingImageLayout);

    // Staging buffer
    magma::vulkan::Capsule<VkBuffer> stagingBuffer{m_device.capsule(), vkDestroyBuffer};
    magma::vulkan::Capsule<VkDeviceMemory> stagingBufferMemory{m_device.capsule(), vkFreeMemory};

    VkDeviceSize imageSize = 4u;
    magma::vulkan::createBuffer(m_device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                                stagingBufferMemory);

    void* data;

    // The real image - plain white
    vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
    memset(data, 0xFF, 4u);
    vkUnmapMemory(m_device, stagingBufferMemory);

    vulkan::createImage(m_device, 1u, 1u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        m_dummyImage, m_dummyImageMemory);

    vulkan::transitionImageLayout(m_device, m_commandPool, m_dummyImage, VK_IMAGE_LAYOUT_PREINITIALIZED,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vulkan::copyBufferToImage(m_device, m_commandPool, stagingBuffer, m_dummyImage, 1u, 1u);

    vulkan::createImageView(m_device, m_dummyImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, m_dummyImageView);

    // The real normal image - plain flat blue
    vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
    memset(data, 0x80, 2u);
    memset(reinterpret_cast<uint8_t*>(data) + 2, 0xFF, 2u);
    vkUnmapMemory(m_device, stagingBufferMemory);

    vulkan::createImage(m_device, 1u, 1u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        m_dummyNormalImage, m_dummyNormalImageMemory);

    vulkan::transitionImageLayout(m_device, m_commandPool, m_dummyNormalImage, VK_IMAGE_LAYOUT_PREINITIALIZED,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vulkan::copyBufferToImage(m_device, m_commandPool, stagingBuffer, m_dummyNormalImage, 1u, 1u);

    vulkan::createImageView(m_device, m_dummyNormalImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT,
                            m_dummyNormalImageView);
}

void RenderEngine::Impl::createTextureSampler()
{
    logger.info("magma.vulkan.render-engine") << "Creating texture sampler." << std::endl;

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16; // Over 16 is useless, but lower that for better performances
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.f;
    samplerInfo.minLod = 0.f;
    samplerInfo.maxLod = 0.f;

    if (vkCreateSampler(m_device, &samplerInfo, nullptr, m_textureSampler.replace()) != VK_SUCCESS) {
        logger.error("magma.vulkan.texture-sampler") << "Failed to texture sampler." << std::endl;
    }
}

void RenderEngine::Impl::createDescriptorPool()
{
    logger.info("magma.vulkan.render-engine") << "Creating descriptor pool." << std::endl;

    std::array<VkDescriptorPoolSize, 1> poolSizes = {};

    {
        // Camera UBO
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = 1;

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = 1;
        poolInfo.flags = 0;

        if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, m_cameraDescriptorPool.replace()) != VK_SUCCESS) {
            logger.error("magma.vulkan.descriptor-pool") << "Failed to create descriptor pool." << std::endl;
            exit(1);
        }
    }

    // @todo Should be somewhere else?
    {
        std::array<VkDescriptorPoolSize, 1> poolSizes = {};

        const uint32_t maxSets = 128u;
        // @todo How to choose? (= max number of meshes)
        // @fixme Understand why we need to multiply all the descriptor counts.

        // Models UBO
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = 1 * maxSets;

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = maxSets;
        poolInfo.flags = 0;

        if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, m_meshDescriptorPool.replace()) != VK_SUCCESS) {
            logger.error("magma.vulkan.descriptor-pool") << "Failed to create mesh descriptor pool." << std::endl;
            exit(1);
        }
    }

    // @todo Should be somewhere else - this closely related to current shader
    {
        std::array<VkDescriptorPoolSize, 2> poolSizes = {};

        const uint32_t maxSets = 128u;
        // @todo How to choose? (= max number of materials)

        // Materials UBO
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = 1 * maxSets;

        // Albedo, normal map and ORM map
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = 3 * maxSets;

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = maxSets;
        poolInfo.flags = 0;

        if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, m_materialDescriptorPool.replace()) != VK_SUCCESS) {
            logger.error("magma.vulkan.descriptor-pool") << "Failed to create material descriptor pool." << std::endl;
            exit(1);
        }
    }
}

VkCommandBuffer& RenderEngine::Impl::recordCommandBuffer(uint32_t index)
{
    vk::CommandBuffer commandBuffer{m_commandBuffers[index]}; // @cleanup HPP

    //----- Prologue

    vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eSimultaneousUse};
    commandBuffer.begin(&beginInfo);

    UserDataRenderIn userData;
    userData.commandBuffer = &commandBuffer;

    //----- G-Buffer

    // @todo We should not need to pass index, this is oonly needed because GBuffer presents to swapchain image views
    m_gBuffer.beginRender(commandBuffer, index);
    userData.pipelineLayout = &m_gBuffer.pipelineLayout();

    // Draw all opaque meshes
    for (auto& camera : m_cameras) {
        camera->render(&userData);
        for (auto& mesh : m_meshes) {
            mesh->render(&userData);
        }

        // @todo Handle multiple cameras?
        // -> Probably not
        break;
    }

    m_gBuffer.endRender(commandBuffer);

    //----- Custom materials

    // @todo userData.pipelineLayout = -> current shader

    //----- Epilogue

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        logger.error("magma.vulkan.command-buffer") << "Failed to record command buffer." << std::endl;
    }

    return m_commandBuffers[index]; // @cleanup HPP
}

void RenderEngine::Impl::createCommandBuffers()
{
    // Free previous command buffers if any
    if (m_commandBuffers.size() > 0) {
        vkFreeCommandBuffers(m_device, m_commandPool, m_commandBuffers.size(), m_commandBuffers.data());
    }

    m_commandBuffers.resize(m_swapchain.imageViews().size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    if (vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
        logger.error("magma.vulkan.command-buffers") << "Failed to create command buffers." << std::endl;
    }

    // Start recording
    for (auto i = 0u; i < m_commandBuffers.size(); i++) {
        recordCommandBuffer(i);
    }
}

void RenderEngine::Impl::createSemaphores()
{
    logger.info("magma.vulkan.render-engine") << "Creating semaphores." << std::endl;

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, m_imageAvailableSemaphore.replace()) != VK_SUCCESS
        || vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, m_renderFinishedSemaphore.replace()) != VK_SUCCESS) {
        logger.error("magma.vulkan.command-buffer") << "Failed to create semaphores." << std::endl;
    }
}

void RenderEngine::Impl::recreateSwapchain()
{
    logger.info("magma.vulkan.render-engine") << "Recreating swapchain." << std::endl;

    vkDeviceWaitIdle(m_device);

    m_swapchain.init(m_surface, m_windowExtent);

    createPipelines();
    createCommandBuffers();
}

void RenderEngine::Impl::initVulkan()
{
    m_instance.init(true);
    m_surface.init(m_windowHandle);
    m_device.init(m_instance.capsule(), m_surface);
    m_swapchain.init(m_surface, m_windowExtent);

    createCommandPool();
    createDescriptorSetLayouts();
    createPipelines();
    createDummyTexture();
    createTextureSampler();
    createDescriptorPool();
    createCommandBuffers();
    createSemaphores();
}
