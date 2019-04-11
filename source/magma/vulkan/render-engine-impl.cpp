#include "./render-engine-impl.hpp"

#include <lava/magma/vr-tools.hpp>

#include "../shmag-reader.hpp"
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
    initVr();
    initVulkan();
}

RenderEngine::Impl::~Impl()
{
    vr::VR_Shutdown();

    device().waitIdle();
}

void RenderEngine::Impl::update()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    // Updating shaders
    while (auto event = m_shadersWatcher.pollEvent()) {
        if (event->type == chamber::FileWatchEvent::Type::Modified) {
            auto materialPath = event->path;
            auto watchId = event->watchId;

            logger.info("magma.render-engine") << "Material source file " << event->path << " has changed." << std::endl;
            logger.log().tab(1);

            auto materialInfo =
                std::find_if(m_materialInfos.begin(), m_materialInfos.end(),
                             [&watchId](const auto& materialInfo) { return materialInfo.second.watchId == watchId; });

            ShmagReader shmagReader(materialPath);
            auto shaderImplementation = shmagReader.processedString();
            if (shmagReader.errored()) {
                logger.warning("magma.vulkan.render-engine")
                    << "Cannot update watched material " << materialInfo->first << ", shmag reading failed." << std::endl;
                return;
            }

            m_shadersManager.updateImplGroup(materialInfo->first, shaderImplementation);

            logger.log().tab(-1);
        }
    }
}

void RenderEngine::Impl::draw()
{
    // @note This function does both render and draw, so no color.
    PROFILE_FUNCTION();

    // We record all render scenes cameras once
    for (auto renderSceneId = 0u; renderSceneId < m_renderScenes.size(); ++renderSceneId) {
        auto& renderScene = m_renderScenes[renderSceneId];
        renderScene->interfaceImpl().record();

        // Tracking
        if (m_logTracking) {
            logger.info("magma.render-engine") << "Render scene " << renderSceneId << "." << std::endl;
            logger.log().tab(1);
            logger.log() << "draw-calls.renderer: " << tracker.counter("draw-calls.renderer") << std::endl;
            logger.log() << "draw-calls.shadows: " << tracker.counter("draw-calls.shadows") << std::endl;
            logger.log().tab(-1);
            m_logTracking = false;
        }
    }

    for (auto renderTargetId = 0u; renderTargetId < m_renderTargetBundles.size(); ++renderTargetId) {
        auto& renderTargetBundle = m_renderTargetBundles[renderTargetId];
        auto& renderTargetImpl = renderTargetBundle.renderTarget->interfaceImpl();
        // const auto& swapchainHolder = renderTargetImpl.swapchainHolder();

        if (!renderTargetImpl.prepare()) continue;

        // Record command buffer each frame
        auto currentIndex = renderTargetImpl.currentBufferIndex();
        const auto& commandBuffers = recordCommandBuffer(renderTargetId, currentIndex);
        renderTargetImpl.draw(commandBuffers);
    }
}

uint32_t RenderEngine::Impl::registerMaterialFromFile(const std::string& hrid, const fs::Path& shaderPath)
{
    PROFILE_FUNCTION(PROFILER_COLOR_REGISTER);

    auto materialId = m_registeredMaterialsMap.size();
    m_registeredMaterialsMap[hrid] = materialId;

    logger.info("magma.vulkan.render-engine") << "Registering material " << hrid << " as " << materialId << "." << std::endl;

    // @note ShaderManager cannot handle shmag directly, it uses @magma:impl thingy
    // to be able to switch-case them or so in other renderer shaders.

    ShmagReader shmagReader(shaderPath);
    auto uniformDefinitions = shmagReader.uniformDefinitions();
    auto shaderImplementation = shmagReader.processedString();
    if (shmagReader.errored()) {
        logger.warning("magma.vulkan.render-engine")
            << "Cannot register material " << hrid << ", shmag reading failed." << std::endl;
        return -1u;
    }

    // Start watching the file
    const auto watchId = m_shadersWatcher.watch(shaderPath);
    auto& materialInfo = m_materialInfos[hrid];
    materialInfo.id = materialId;
    materialInfo.uniformDefinitions = uniformDefinitions;
    materialInfo.sourcePath = shaderPath;
    materialInfo.watchId = watchId;

    m_shadersManager.registerImplGroup(hrid, shaderImplementation);

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

    PROFILE_FUNCTION(PROFILER_COLOR_REGISTER);

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

    // @fixme Should be "compositorViewId"...
    renderView.presentViewId =
        renderTargetBundle.renderTarget->interfaceImpl().addView(imageView, imageLayout, m_dummySampler, viewport);

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

    PROFILE_FUNCTION(PROFILER_COLOR_REGISTER);

    auto& renderView = m_renderViews[viewId];
    auto& renderTargetBundle = m_renderTargetBundles[renderView.renderTargetId];
    renderTargetBundle.renderTarget->interfaceImpl().removeView(renderView.presentViewId);
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
    // So we init what's left of the engine right here, when adding a renderTarget.
    if (!m_deviceHolder.device()) {
        // @fixme We never call it with a surface, so all that thingy thing might not be useful...
        // But we *should* use it... Otherwise the selected queueFamily might not be adapted.
        // To do so, we need a flag on RenderEngine creation (usage: Vr, Window, Offscreen).
        // We would then init with surface only when a WindowRenderTarget is created,
        // if the Window flag is on.
        initVulkanDevice(nullptr);
        // initVulkanDevice(&renderTargetImpl.surface());
    }

    uint32_t renderTargetId = m_renderTargetBundles.size();
    renderTargetImpl.init(renderTargetId);

    m_renderTargetBundles.emplace_back();
    auto& renderTargetBundle = m_renderTargetBundles.back();
    renderTargetBundle.renderTarget = std::move(renderTarget);

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
        renderTargetBundle.renderTarget->interfaceImpl().updateView(renderView.presentViewId, imageView, imageLayout,
                                                                    m_dummySampler);
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

void RenderEngine::Impl::createCommandPool(vk::SurfaceKHR* pSurface)
{
    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

    logger.info("magma.vulkan.render-engine") << "Creating command pool." << std::endl;

    auto queueFamilyIndices = vulkan::findQueueFamilies(physicalDevice(), pSurface);

    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphics;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    if (device().createCommandPool(&poolInfo, nullptr, m_commandPool.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine") << "Failed to create command pool." << std::endl;
    }
}

void RenderEngine::Impl::createDummyTextures()
{
    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

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

std::vector<vk::CommandBuffer> RenderEngine::Impl::recordCommandBuffer(uint32_t renderTargetId, uint32_t bufferIndex)
{
    PROFILE_FUNCTION(PROFILER_COLOR_RENDER);

    auto& renderTargetBundle = m_renderTargetBundles[renderTargetId];

    if (bufferIndex >= renderTargetBundle.commandBuffers.size()) {
        logger.error("magma.vulkan.render-engine")
            << "Invalid bufferIndex during command buffers recording (" << bufferIndex << ") that should have been between 0 and "
            << renderTargetBundle.commandBuffers.size() - 1u << "." << std::endl;
    }

    //----- Render all scenes

    // @fixme Feels like we should be able to generate all render scenes images,
    // and use these directly without having to re-render everything.
    std::vector<vk::CommandBuffer> commandBuffers;

    for (auto renderSceneId = 0u; renderSceneId < m_renderScenes.size(); ++renderSceneId) {
        auto& renderScene = m_renderScenes[renderSceneId];
        const auto& sceneCommandBuffers = renderScene->interfaceImpl().commandBuffers();
        commandBuffers.insert(commandBuffers.end(), sceneCommandBuffers.begin(), sceneCommandBuffers.end());
    }

    //----- Render targets' specific rendering

    auto& commandBuffer = renderTargetBundle.commandBuffers[bufferIndex];

    vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eSimultaneousUse};
    commandBuffer.begin(&beginInfo);

    // @todo The names are unclear, what's the difference between render and draw?
    // @fixme We probably don't need handle a render target command buffer by ourself
    // because this line is the only thing we do with it.
    renderTargetBundle.renderTarget->interfaceImpl().render(commandBuffer);

    commandBuffer.end();

    commandBuffers.emplace_back(commandBuffer);

    return commandBuffers;
}

void RenderEngine::Impl::createCommandBuffers(uint32_t renderTargetId)
{
    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

    logger.log() << "Creating command buffers for render target " << renderTargetId << "." << std::endl;

    auto& renderTargetBundle = m_renderTargetBundles[renderTargetId];
    auto& commandBuffers = renderTargetBundle.commandBuffers;

    // Free previous command buffers if any
    if (commandBuffers.size() > 0) {
        device().freeCommandBuffers(m_commandPool, commandBuffers.size(), commandBuffers.data());
    }

    // Allocate them all
    commandBuffers.resize(renderTargetBundle.renderTarget->interfaceImpl().buffersCount());

    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.commandPool = m_commandPool;
    allocateInfo.level = vk::CommandBufferLevel::ePrimary;
    allocateInfo.commandBufferCount = commandBuffers.size();

    if (device().allocateCommandBuffers(&allocateInfo, commandBuffers.data()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine") << "Failed to create command buffers." << std::endl;
    }
}

void RenderEngine::Impl::initRenderScenes()
{
    if (m_renderScenes.size() == 0u) return;

    PROFILE_FUNCTION(PROFILER_COLOR_INIT);

    logger.info("magma.vulkan.render-engine") << "Initializing render scenes." << std::endl;
    logger.log().tab(1);

    for (auto renderSceneId = 0u; renderSceneId < m_renderScenes.size(); ++renderSceneId) {
        auto& renderSceneImpl = m_renderScenes[renderSceneId]->interfaceImpl();
        renderSceneImpl.init(renderSceneId);
    }

    logger.log().tab(-1);
}

void RenderEngine::Impl::initVr()
{
    PROFILE_FUNCTION(PROFILER_COLOR_INIT);

    logger.info("magma.vulkan.render-engine") << "Initializing VR." << std::endl;
    logger.log().tab(1);

    // @note This VR initialisation has to be done before initVulkan, because we need
    // to get the extensions list to be enable.
    if (vrAvailable()) {
        // Initializing VR system
        vr::EVRInitError error = vr::VRInitError_None;
        m_vrSystem = vr::VR_Init(&error, vr::EVRApplicationType::VRApplication_Scene);

        if (error != vr::VRInitError_None) {
            logger.warning("magma.vulkan.render-engine")
                << "VR seems available but we failed to init VR. Is SteamVR ready?" << std::endl;
        }
    }
    else {
        logger.log() << "VR is not available." << std::endl;
    }

    logger.log().tab(-1);
}

void RenderEngine::Impl::initVulkan()
{
    PROFILE_FUNCTION(PROFILER_COLOR_INIT);

    logger.info("magma.vulkan.render-engine") << "Initializing vulkan." << std::endl;
    logger.log().tab(1);

    m_instanceHolder.init(true, vrEnabled());

    logger.log().tab(-1);
}

void RenderEngine::Impl::initVulkanDevice(vk::SurfaceKHR* pSurface)
{
    PROFILE_FUNCTION(PROFILER_COLOR_INIT);

    logger.info("magma.vulkan.render-engine") << "Initializing vulkan device." << std::endl;
    logger.log().tab(1);

    m_deviceHolder.init(instance(), pSurface, m_instanceHolder.debugEnabled(), m_instanceHolder.vrEnabled());

    createCommandPool(pSurface);
    createDummyTextures();

    initRenderScenes();

    logger.log().tab(-1);
}
