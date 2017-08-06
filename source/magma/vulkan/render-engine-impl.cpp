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
#include "./vertex.hpp"
#include "./wrappers.hpp"

using namespace lava::magma;
using namespace lava::chamber;

RenderEngine::Impl::Impl()
{
    initVulkan();
}

RenderEngine::Impl::~Impl()
{
    vkDeviceWaitIdle(m_device);
}

void RenderEngine::Impl::draw()
{
    if (m_renderTargetBundles.size() != 1u) {
        logger.error("magma.vulkan.render-engine") << "No or too many render targets added during draw." << std::endl;
    }

    auto& renderTargetBundle = m_renderTargetBundles[0];
    auto& renderTarget = *renderTargetBundle.renderTarget;
    const auto& data = renderTargetBundle.data();

    renderTarget.prepare();

    // Record command buffer each frame
    vkDeviceWaitIdle(m_device); // @todo Better wait for a fence on the queue
    auto& commandBuffer = recordCommandBuffer(0, data.swapchainHolder.currentIndex());

    // Submit it to the queue
    vk::Semaphore waitSemaphores[] = {data.swapchainHolder.imageAvailableSemaphore()};
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    vk::SubmitInfo submitInfo;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_renderFinishedSemaphore;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // @cleanup HPP
    if (vkQueueSubmit(m_device.graphicsQueue(), 1, reinterpret_cast<VkSubmitInfo*>(&submitInfo), VK_NULL_HANDLE) != VK_SUCCESS) {
        logger.error("magma.vulkan.layer") << "Failed to submit draw command buffer." << std::endl;
    }

    InDataRenderTargetDraw drawData{m_renderFinishedSemaphore};
    renderTarget.draw(&drawData);
}

void RenderEngine::Impl::update()
{
}

void RenderEngine::Impl::add(std::unique_ptr<IRenderTarget>&& renderTarget)
{
    logger.info("magma.vulkan.render-engine") << "Adding render target " << m_renderTargetBundles.size() << "." << std::endl;
    logger.log().tab(1);

    const auto& data = *reinterpret_cast<const DataRenderTarget*>(renderTarget->data());

    // @todo This is probably not the right thing to do.
    // However - what can be the solution?
    // Device creation requires a surface, which requires a windowHandle.
    // Thus, no window => unable to init the device.
    // So we init what's left of the engine write here, when adding a renderTarget.
    {
        // @cleanup HPP
        const auto& surface = reinterpret_cast<const VkSurfaceKHR&>(data.surface);
        initVulkanDevice(surface); // As the render target below must have a valid device.
    }

    renderTarget->init();

    RenderTargetBundle renderTargetBundle;
    renderTargetBundle.renderTarget = std::move(renderTarget);
    renderTargetBundle.presentStage = std::make_unique<Present>(*this);
    renderTargetBundle.presentStage->bindSwapchainHolder(data.swapchainHolder);
    renderTargetBundle.presentStage->init();
    renderTargetBundle.presentStage->imageView(m_epiphany.imageView(), m_dummySampler);
    renderTargetBundle.presentStage->update(data.swapchainHolder.extent());
    m_renderTargetBundles.emplace_back(std::move(renderTargetBundle));

    createCommandBuffers(m_renderTargetBundles.size() - 1u);

    logger.log().tab(-1);
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

void RenderEngine::Impl::initStages()
{
    logger.info("magma.vulkan.render-engine") << "Initializing render stages." << std::endl;
    logger.log().tab(1);

    m_gBuffer.init();
    m_epiphany.init();

    logger.log().tab(-1);
}

void RenderEngine::Impl::updateStages()
{
    logger.info("magma.vulkan.render-engine") << "Updating render stages." << std::endl;
    logger.log().tab(1);

    // @todo The extent should be the one defined in the scene containing this
    auto extent = vk::Extent2D(800u, 600u);
    m_gBuffer.update(extent);
    m_epiphany.update(extent);

    // Set-up
    m_epiphany.normalImageView(m_gBuffer.normalImageView(), m_dummySampler);
    m_epiphany.albedoImageView(m_gBuffer.albedoImageView(), m_dummySampler);
    m_epiphany.ormImageView(m_gBuffer.ormImageView(), m_dummySampler);
    m_epiphany.depthImageView(m_gBuffer.depthImageView(), m_dummySampler);

    logger.log().tab(-1);
}

void RenderEngine::Impl::createCommandPool(VkSurfaceKHR surface)
{
    logger.info("magma.vulkan.render-engine") << "Creating command pool." << std::endl;

    auto queueFamilyIndices = vulkan::findQueueFamilies(m_device.physicalDevice(), surface);

    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphics;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    // @cleanup HPP
    if (m_device.vk().createCommandPool(&poolInfo, nullptr, m_commandPool.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine") << "Failed to create command pool." << std::endl;
    }
}

void RenderEngine::Impl::createDummyTextures()
{
    logger.info("magma.vulkan.render-engine") << "Creating dummy textures." << std::endl;

    // Plain white
    std::vector<uint8_t> dummyData = {0xFF, 0xFF, 0xFF, 0xFF};
    m_dummyImageHolder.setup(dummyData, 1, 1, 4);

    // Flat blue
    dummyData = {0x80, 0x80, 0xFF, 0xFF};
    m_dummyNormalImageHolder.setup(dummyData, 1, 1, 4);

    // Sampler
    vk::SamplerCreateInfo samplerInfo;
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable = true;
    samplerInfo.maxAnisotropy = 16; // Over 16 is useless, but lower that for better performances
    samplerInfo.unnormalizedCoordinates = false;
    samplerInfo.compareEnable = false;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;

    // @cleanup HPP
    const auto& vk_device = m_device.vk();

    if (vk_device.createSampler(&samplerInfo, nullptr, m_dummySampler.replace()) != vk::Result::eSuccess) {
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

vk::CommandBuffer& RenderEngine::Impl::recordCommandBuffer(uint32_t renderTargetIndex, uint32_t bufferIndex)
{

    auto& renderTargetBundle = m_renderTargetBundles[renderTargetIndex];

    if (bufferIndex >= renderTargetBundle.commandBuffers.size()) {
        logger.error("magma.vulkan.render-engine")
            << "Invalid bufferIndex during command buffers recording (" << bufferIndex << ") that should have been between 0 and "
            << renderTargetBundle.commandBuffers.size() - 1u << "." << std::endl;
    }

    auto& commandBuffer = renderTargetBundle.commandBuffers[bufferIndex];

    //----- Prologue

    vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eSimultaneousUse};
    commandBuffer.begin(&beginInfo);

    //----- Render

    m_gBuffer.render(commandBuffer);
    m_epiphany.render(commandBuffer);
    renderTargetBundle.presentStage->render(commandBuffer);

    //----- Epilogue

    commandBuffer.end();

    return commandBuffer;
}

void RenderEngine::Impl::createCommandBuffers(uint32_t renderTargetIndex)
{
    logger.log() << "Creating command buffers for render target " << renderTargetIndex << "." << std::endl;

    // @cleanup HPP
    auto& vk_device = m_device.vk();

    auto& renderTargetBundle = m_renderTargetBundles[renderTargetIndex];
    auto& commandBuffers = renderTargetBundle.commandBuffers;
    auto& swapchain = renderTargetBundle.data().swapchainHolder;

    // Free previous command buffers if any
    if (commandBuffers.size() > 0) {
        vk_device.freeCommandBuffers(m_commandPool, commandBuffers.size(), commandBuffers.data());
    }

    // Allocate them all
    commandBuffers.resize(swapchain.imagesCount());

    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.commandPool = m_commandPool;
    allocateInfo.level = vk::CommandBufferLevel::ePrimary;
    allocateInfo.commandBufferCount = commandBuffers.size();

    if (vk_device.allocateCommandBuffers(&allocateInfo, commandBuffers.data()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.command-buffers") << "Failed to create command buffers." << std::endl;
    }
}

void RenderEngine::Impl::createSemaphores()
{
    logger.info("magma.vulkan.render-engine") << "Creating semaphores." << std::endl;

    // @cleanup HPP
    const auto& vk_device = m_device.vk();

    vk::SemaphoreCreateInfo semaphoreInfo;

    if (vk_device.createSemaphore(&semaphoreInfo, nullptr, m_renderFinishedSemaphore.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine") << "Failed to create semaphores." << std::endl;
    }
}

void RenderEngine::Impl::initVulkan()
{
    logger.info("magma.vulkan.render-engine") << "Initializing vulkan." << std::endl;
    logger.log().tab(1);

    m_instance.init(true);

    logger.log().tab(-1);
}

void RenderEngine::Impl::initVulkanDevice(VkSurfaceKHR surface)
{
    logger.info("magma.vulkan.render-engine") << "Initializing vulkan device." << std::endl;
    logger.log().tab(1);

    m_device.init(m_instance.capsule(), surface);

    createCommandPool(surface);
    createDescriptorSetLayouts();
    createDummyTextures();
    initStages();
    updateStages();
    createDescriptorPool();
    createSemaphores();

    logger.log().tab(-1);
}
