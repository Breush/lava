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
    device().waitIdle();
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
    device().waitIdle(); // @todo Better wait for a fence on the queue
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

    if (graphicsQueue().submit(1, &submitInfo, nullptr) != vk::Result::eSuccess) {
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
        initVulkanDevice(data.surface); // As the render target below must have a valid device.
    }

    InDataRenderTargetInit dataRenderTargetInit;
    dataRenderTargetInit.id = m_renderTargetBundles.size();
    renderTarget->init(&dataRenderTargetInit);

    RenderTargetBundle renderTargetBundle;
    renderTargetBundle.renderTarget = std::move(renderTarget);
    renderTargetBundle.presentStage = std::make_unique<Present>(*this);
    renderTargetBundle.presentStage->bindSwapchainHolder(data.swapchainHolder);
    renderTargetBundle.presentStage->init();
    renderTargetBundle.presentStage->imageView(m_epiphany.imageView(), m_dummySampler);
    m_renderTargetBundles.emplace_back(std::move(renderTargetBundle));

    updateRenderTarget(dataRenderTargetInit.id);
    createCommandBuffers(dataRenderTargetInit.id);

    logger.log().tab(-1);
}

void RenderEngine::Impl::updateRenderTarget(uint32_t renderTargetId)
{
    if (renderTargetId > m_renderTargetBundles.size()) {
        logger.warning("magma.vulkan.render-engine")
            << "Updating render target with invalid id: " << renderTargetId << "." << std::endl;
        return;
    }

    auto& renderTargetBundle = m_renderTargetBundles[renderTargetId];
    const auto& data = renderTargetBundle.data();
    renderTargetBundle.presentStage->update(data.swapchainHolder.extent());
}

void RenderEngine::Impl::createDescriptorSetLayouts()
{
    logger.info("magma.vulkan.render-engine") << "Creating descriptor set layouts." << std::endl;

    {
        // Camera UBO
        vk::DescriptorSetLayoutBinding transformsLayoutBinding;
        transformsLayoutBinding.binding = 0;
        transformsLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
        transformsLayoutBinding.descriptorCount = 1;
        transformsLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

        vk::DescriptorSetLayoutCreateInfo layoutInfo;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &transformsLayoutBinding;

        if (device().createDescriptorSetLayout(&layoutInfo, nullptr, m_cameraDescriptorSetLayout.replace())
            != vk::Result::eSuccess) {
            logger.error("magma.vulkan.descriptor-set-layout") << "Failed to create descriptor set layout." << std::endl;
        }
    }

    // @todo Move ?
    {
        // Model UBO
        vk::DescriptorSetLayoutBinding modelLayoutBinding;
        modelLayoutBinding.binding = 0;
        modelLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
        modelLayoutBinding.descriptorCount = 1;
        modelLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

        vk::DescriptorSetLayoutCreateInfo layoutInfo;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &modelLayoutBinding;

        if (device().createDescriptorSetLayout(&layoutInfo, nullptr, m_meshDescriptorSetLayout.replace())
            != vk::Result::eSuccess) {
            logger.error("magma.vulkan.descriptor-set-layout") << "Failed to create mesh descriptor set layout." << std::endl;
        }
    }

    // @todo Move, this is very individual shader-related
    // @note Not so much, as this is what's REQUIRED by G-Buffer
    {
        // Attributes UBO
        vk::DescriptorSetLayoutBinding attributesLayoutBinding;
        attributesLayoutBinding.binding = 0;
        attributesLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
        attributesLayoutBinding.descriptorCount = 1;
        attributesLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

        // Sampler
        vk::DescriptorSetLayoutBinding normalMapLayoutBinding;
        normalMapLayoutBinding.binding = 1;
        normalMapLayoutBinding.descriptorCount = 1;
        normalMapLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        normalMapLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

        // Sampler
        vk::DescriptorSetLayoutBinding baseColorLayoutBinding;
        baseColorLayoutBinding.binding = 2;
        baseColorLayoutBinding.descriptorCount = 1;
        baseColorLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        baseColorLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

        // Sampler
        vk::DescriptorSetLayoutBinding ormLayoutBinding;
        ormLayoutBinding.binding = 3;
        ormLayoutBinding.descriptorCount = 1;
        ormLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        ormLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

        std::array<vk::DescriptorSetLayoutBinding, 4> bindings = {attributesLayoutBinding, baseColorLayoutBinding,
                                                                  normalMapLayoutBinding, ormLayoutBinding};
        vk::DescriptorSetLayoutCreateInfo layoutInfo;
        layoutInfo.bindingCount = bindings.size();
        layoutInfo.pBindings = bindings.data();

        if (device().createDescriptorSetLayout(&layoutInfo, nullptr, m_materialDescriptorSetLayout.replace())
            != vk::Result::eSuccess) {
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

void RenderEngine::Impl::createCommandPool(vk::SurfaceKHR surface)
{
    logger.info("magma.vulkan.render-engine") << "Creating command pool." << std::endl;

    auto queueFamilyIndices = vulkan::findQueueFamilies(physicalDevice(), surface);

    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphics;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    if (device().createCommandPool(&poolInfo, nullptr, m_commandPool.replace()) != vk::Result::eSuccess) {
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

    if (device().createSampler(&samplerInfo, nullptr, m_dummySampler.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.texture-sampler") << "Failed to texture sampler." << std::endl;
    }
}

void RenderEngine::Impl::createDescriptorPool()
{
    logger.info("magma.vulkan.render-engine") << "Creating descriptor pool." << std::endl;

    {
        // Camera UBO
        std::array<vk::DescriptorPoolSize, 1> poolSizes;
        poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
        poolSizes[0].descriptorCount = 1;

        vk::DescriptorPoolCreateInfo poolInfo;
        poolInfo.poolSizeCount = poolSizes.size();
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = 1;

        if (device().createDescriptorPool(&poolInfo, nullptr, m_cameraDescriptorPool.replace()) != vk::Result::eSuccess) {
            logger.error("magma.vulkan.descriptor-pool") << "Failed to create descriptor pool." << std::endl;
        }
    }

    // @todo Should be somewhere else?
    {
        // @todo How to choose? (= max number of meshes)
        const uint32_t maxSets = 128u;

        // Models UBO
        std::array<vk::DescriptorPoolSize, 1> poolSizes = {};
        poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
        poolSizes[0].descriptorCount = 1 * maxSets;

        vk::DescriptorPoolCreateInfo poolInfo;
        poolInfo.poolSizeCount = poolSizes.size();
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = maxSets;

        if (device().createDescriptorPool(&poolInfo, nullptr, m_meshDescriptorPool.replace()) != vk::Result::eSuccess) {
            logger.error("magma.vulkan.descriptor-pool") << "Failed to create mesh descriptor pool." << std::endl;
        }
    }

    // @todo Should be somewhere else - this is closely related to current shader
    {
        // @todo How to choose? (= max number of materials)
        const uint32_t maxSets = 128u;

        // Materials UBO
        std::array<vk::DescriptorPoolSize, 2> poolSizes = {};
        poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
        poolSizes[0].descriptorCount = 1 * maxSets;

        // Albedo, normal map and ORM map
        poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
        poolSizes[1].descriptorCount = 3 * maxSets;

        vk::DescriptorPoolCreateInfo poolInfo;
        poolInfo.poolSizeCount = poolSizes.size();
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = maxSets;

        if (device().createDescriptorPool(&poolInfo, nullptr, m_materialDescriptorPool.replace()) != vk::Result::eSuccess) {
            logger.error("magma.vulkan.descriptor-pool") << "Failed to create material descriptor pool." << std::endl;
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

    auto& renderTargetBundle = m_renderTargetBundles[renderTargetIndex];
    auto& commandBuffers = renderTargetBundle.commandBuffers;
    auto& swapchain = renderTargetBundle.data().swapchainHolder;

    // Free previous command buffers if any
    if (commandBuffers.size() > 0) {
        device().freeCommandBuffers(m_commandPool, commandBuffers.size(), commandBuffers.data());
    }

    // Allocate them all
    commandBuffers.resize(swapchain.imagesCount());

    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.commandPool = m_commandPool;
    allocateInfo.level = vk::CommandBufferLevel::ePrimary;
    allocateInfo.commandBufferCount = commandBuffers.size();

    if (device().allocateCommandBuffers(&allocateInfo, commandBuffers.data()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.command-buffers") << "Failed to create command buffers." << std::endl;
    }
}

void RenderEngine::Impl::createSemaphores()
{
    logger.info("magma.vulkan.render-engine") << "Creating semaphores." << std::endl;

    vk::SemaphoreCreateInfo semaphoreInfo;

    if (device().createSemaphore(&semaphoreInfo, nullptr, m_renderFinishedSemaphore.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine") << "Failed to create semaphores." << std::endl;
    }
}

void RenderEngine::Impl::initVulkan()
{
    logger.info("magma.vulkan.render-engine") << "Initializing vulkan." << std::endl;
    logger.log().tab(1);

    m_instanceHolder.init(true);

    logger.log().tab(-1);
}

void RenderEngine::Impl::initVulkanDevice(vk::SurfaceKHR surface)
{
    logger.info("magma.vulkan.render-engine") << "Initializing vulkan device." << std::endl;
    logger.log().tab(1);

    m_deviceHolder.init(instance(), surface);

    createCommandPool(surface);
    createDescriptorSetLayouts();
    createDummyTextures();
    initStages();
    updateStages();
    createDescriptorPool();
    createSemaphores();

    logger.log().tab(-1);
}
