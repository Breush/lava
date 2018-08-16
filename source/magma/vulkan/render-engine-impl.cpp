#include "./render-engine-impl.hpp"

#include <chrono>
#include <fstream>
#include <lava/chamber/logger.hpp>
#include <sstream>

#include "./cameras/i-camera-impl.hpp"
#include "./helpers/queue.hpp"
#include "./holders/swapchain-holder.hpp"
#include "./render-image-impl.hpp"
#include "./render-scenes/i-render-scene-impl.hpp"
#include "./render-targets/i-render-target-impl.hpp"
#include "./stages/present.hpp"

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

void RenderEngine::Impl::update()
{
    // Updating shaders
    while (auto event = m_shadersWatcher.pollEvent()) {
        if (event->type == chamber::FileWatchEvent::Type::Modified) {
            auto materialPath = event->path;
            auto watchId = event->watchId;

            logger.info("magma.render-engine") << "Material source file " << event->path << " has changed." << std::endl;
            logger.log().tab(1);

            std::ifstream fileStream(materialPath.string());
            std::stringstream buffer;
            buffer << fileStream.rdbuf();

            auto materialInfo =
                std::find_if(m_materialInfos.begin(), m_materialInfos.end(),
                             [&watchId](const auto& materialInfo) { return materialInfo.second.watchId == watchId; });

            m_shadersManager.updateImplGroup(materialInfo->first, buffer.str());

            logger.log().tab(-1);
        }
    }
}

void RenderEngine::Impl::draw()
{
    for (auto renderTargetId = 0u; renderTargetId < m_renderTargetBundles.size(); ++renderTargetId) {
        auto& renderTargetBundle = m_renderTargetBundles[renderTargetId];
        auto& renderTargetImpl = renderTargetBundle.renderTarget->interfaceImpl();
        const auto& swapchainHolder = renderTargetImpl.swapchainHolder();

        renderTargetImpl.prepare();

        // Record command buffer each frame
        m_deviceHolder.device().waitIdle(); // @todo Better wait for a fence on each queue
        auto& commandBuffer = recordCommandBuffer(renderTargetId, swapchainHolder.currentIndex());

        // Submit it to the queue
        vk::Semaphore waitSemaphores[] = {swapchainHolder.imageAvailableSemaphore()};
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
            logger.error("magma.vulkan.render-engine") << "Failed to submit draw command buffer." << std::endl;
        }

        renderTargetImpl.draw(m_renderFinishedSemaphore);
    }
}

uint32_t RenderEngine::Impl::registerMaterial(const std::string& hrid, const std::string& shaderImplementation,
                                              const UniformDefinitions& uniformDefinitions)
{
    auto materialId = m_registeredMaterialsMap.size();

    logger.info("magma.vulkan.render-engine") << "Registering material " << hrid << " as " << materialId << "." << std::endl;

    m_registeredMaterialsMap[hrid] = materialId;

    m_shadersManager.registerImplGroup(hrid, shaderImplementation);
    auto& materialInfo = m_materialInfos[hrid];
    materialInfo.id = materialId;
    materialInfo.uniformDefinitions = uniformDefinitions;
    return materialId;
}

uint32_t RenderEngine::Impl::registerMaterialFromFile(const std::string& hrid, const fs::Path& shaderPath,
                                                      const UniformDefinitions& uniformDefinitions)
{
    // Read the file
    std::ifstream fileStream(shaderPath.string());
    std::stringstream buffer;
    buffer << fileStream.rdbuf();

    // Start watching the file
    const auto materialId = registerMaterial(hrid, buffer.str(), uniformDefinitions);
    const auto watchId = m_shadersWatcher.watch(shaderPath);
    m_materialInfos[hrid].sourcePath = shaderPath;
    m_materialInfos[hrid].watchId = watchId;

    return materialId;
}

uint32_t RenderEngine::Impl::addView(RenderImage renderImage, IRenderTarget& renderTarget, Viewport viewport)
{
    const auto& renderImageImpl = renderImage.impl();
    if (renderImageImpl.uuid() == 0u) {
        logger.warning("magma.vulkan.render-engine") << "Trying to add a RenderImage with no uuid." << std::endl;
        return -1u;
    }
    if (!renderImageImpl.view()) {
        logger.warning("magma.vulkan.render-engine") << "Trying to add an empty RenderImage as a view." << std::endl;
        return -1u;
    }

    // Find the render target bundle
    uint32_t renderTargetId = renderTarget.interfaceImpl().id();
    auto& renderTargetBundle = m_renderTargetBundles[renderTargetId];

    // Create the new render view
    m_renderViews.emplace_back();
    auto& renderView = m_renderViews.back();
    renderView.renderImage = renderImage;
    renderView.renderTargetId = renderTargetId;

    // Add a new image to the present stage
    auto imageView = renderImageImpl.view();
    auto imageLayout = renderImageImpl.layout();
    renderView.presentViewId = renderTargetBundle.presentStage->addView(imageView, imageLayout, m_dummySampler, viewport);

    return m_renderViews.size() - 1u;
}

void RenderEngine::Impl::removeView(uint32_t viewId)
{
    // @todo Handle views individually, generating a really unique id
    // that has nothing to do with internal storage.
    if (viewId != m_renderViews.size() - 1u) {
        logger.warning("magma.vulkan.render-engine")
            << "Currently unable to remove a view that is not the last one added." << std::endl;
        return;
    }

    auto& renderView = m_renderViews[viewId];
    auto& renderTargetBundle = m_renderTargetBundles[renderView.renderTargetId];
    renderTargetBundle.presentStage->removeView(renderView.presentViewId);
    m_renderViews.erase(std::begin(m_renderViews) + viewId);
}

//----- Adders

void RenderEngine::Impl::add(std::unique_ptr<IRenderScene>&& renderScene)
{
    const uint32_t renderSceneId = m_renderScenes.size();

    logger.info("magma.vulkan.render-engine") << "Adding render scene " << renderSceneId << "." << std::endl;
    logger.log().tab(1);

    // If no device yet, the scene initialization will be postponed
    // until it is created.
    if (m_deviceHolder.device()) {
        renderScene->interfaceImpl().init(renderSceneId);
    }

    m_renderScenes.emplace_back(std::move(renderScene));

    logger.log().tab(-1);
}

void RenderEngine::Impl::add(std::unique_ptr<IRenderTarget>&& renderTarget)
{
    logger.info("magma.vulkan.render-engine") << "Adding render target " << m_renderTargetBundles.size() << "." << std::endl;
    logger.log().tab(1);

    auto& renderTargetImpl = renderTarget->interfaceImpl();

    // @todo This is probably not the right thing to do.
    // However - what can be the solution?
    // Device creation requires a surface, which requires a handle.
    // Thus, no window => unable to init the device.
    // So we init what's left of the engine write here, when adding a renderTarget.
    if (!m_deviceHolder.device()) {
        initVulkanDevice(renderTargetImpl.surface());
    }

    uint32_t renderTargetId = m_renderTargetBundles.size();
    renderTargetImpl.init(renderTargetId);

    const auto& swapchainHolder = renderTargetImpl.swapchainHolder();
    m_renderTargetBundles.emplace_back();
    auto& renderTargetBundle = m_renderTargetBundles.back();
    renderTargetBundle.renderTarget = std::move(renderTarget);
    renderTargetBundle.presentStage = std::make_unique<Present>(*this);
    renderTargetBundle.presentStage->bindSwapchainHolder(swapchainHolder);
    renderTargetBundle.presentStage->init();
    renderTargetBundle.presentStage->update(swapchainHolder.extent());

    createCommandBuffers(renderTargetId);

    logger.log().tab(-1);
}

void RenderEngine::Impl::updateRenderViews(RenderImage renderImage)
{
    auto& renderImageImpl = renderImage.impl();
    if (renderImageImpl.uuid() == 0u) {
        logger.warning("magma.vulkan.render-engine") << "Updating render views for an unset RenderImage uuid." << std::endl;
        return;
    }

    // The renderImage has changed, we update all image views we were using from it
    for (auto& renderView : m_renderViews) {
        if (renderView.renderImage.impl().uuid() != renderImageImpl.uuid()) continue;
        auto& renderTargetBundle = m_renderTargetBundles[renderView.renderTargetId];

        auto imageView = renderImageImpl.view();
        auto imageLayout = renderImageImpl.layout();
        renderTargetBundle.presentStage->updateView(renderView.presentViewId, imageView, imageLayout, m_dummySampler);
    }
}

const MaterialInfo& RenderEngine::Impl::materialInfo(const std::string& hrid) const
{
    const auto iMaterialInfo = m_materialInfos.find(hrid);
    if (iMaterialInfo == m_materialInfos.end()) {
        logger.error("magma.vulkan.render-engine") << "Material '" << hrid << "' has not been registered." << std::endl;
    }
    return iMaterialInfo->second;
}

void RenderEngine::Impl::updateRenderTarget(uint32_t renderTargetId)
{
    if (renderTargetId > m_renderTargetBundles.size()) {
        logger.warning("magma.vulkan.render-engine")
            << "Updating render target with invalid id: " << renderTargetId << "." << std::endl;
        return;
    }

    auto& renderTargetBundle = m_renderTargetBundles[renderTargetId];
    const auto& swapchainHolder = renderTargetBundle.renderTarget->interfaceImpl().swapchainHolder();

    renderTargetBundle.presentStage->update(swapchainHolder.extent());
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

    // Full zeros
    dummyData = {0x00, 0x00, 0x00, 0x00};
    m_dummyInvisibleImageHolder.setup(dummyData, 1, 1, 4);

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
        logger.error("magma.vulkan.render-engine") << "Failed to create dummy sampler." << std::endl;
    }

    // Shadows sampler
    samplerInfo.magFilter = vk::Filter::eNearest;
    samplerInfo.minFilter = vk::Filter::eNearest;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToBorder;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToBorder;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToBorder;
    samplerInfo.anisotropyEnable = false;
    samplerInfo.unnormalizedCoordinates = false;
    samplerInfo.compareEnable = false;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;

    if (device().createSampler(&samplerInfo, nullptr, m_shadowsSampler.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine") << "Failed to create shadows sampler." << std::endl;
    }
}

vk::CommandBuffer& RenderEngine::Impl::recordCommandBuffer(uint32_t renderTargetId, uint32_t bufferIndex)
{
    auto& renderTargetBundle = m_renderTargetBundles[renderTargetId];

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

    // @todo The scenes should know which cameras to render at any time.
    // And render only those.
    for (auto& renderScene : m_renderScenes) {
        renderScene->interfaceImpl().render(commandBuffer);
    }

    renderTargetBundle.presentStage->render(commandBuffer);

    //----- Epilogue

    commandBuffer.end();

    return commandBuffer;
}

void RenderEngine::Impl::createCommandBuffers(uint32_t renderTargetId)
{
    logger.log() << "Creating command buffers for render target " << renderTargetId << "." << std::endl;

    auto& renderTargetBundle = m_renderTargetBundles[renderTargetId];
    auto& commandBuffers = renderTargetBundle.commandBuffers;

    // Free previous command buffers if any
    if (commandBuffers.size() > 0) {
        device().freeCommandBuffers(m_commandPool, commandBuffers.size(), commandBuffers.data());
    }

    // Allocate them all
    const auto& swapchainHolder = renderTargetBundle.renderTarget->interfaceImpl().swapchainHolder();
    commandBuffers.resize(swapchainHolder.imagesCount());

    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.commandPool = m_commandPool;
    allocateInfo.level = vk::CommandBufferLevel::ePrimary;
    allocateInfo.commandBufferCount = commandBuffers.size();

    if (device().allocateCommandBuffers(&allocateInfo, commandBuffers.data()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.command-buffers") << "Failed to create command buffers." << std::endl;
    }

    for (auto& commandBuffer : commandBuffers) {
        m_deviceHolder.debugMarkerSetObjectName(commandBuffer, "Primary command buffer");
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

void RenderEngine::Impl::initRenderScenes()
{
    if (m_renderScenes.size() == 0u) return;

    logger.info("magma.vulkan.render-engine") << "Initializing render scenes." << std::endl;
    logger.log().tab(1);

    for (auto renderSceneId = 0u; renderSceneId < m_renderScenes.size(); ++renderSceneId) {
        auto& renderSceneImpl = m_renderScenes[renderSceneId]->interfaceImpl();
        renderSceneImpl.init(renderSceneId);
    }

    logger.log().tab(-1);
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
    createDummyTextures();
    createSemaphores();

    initRenderScenes();

    logger.log().tab(-1);
}
