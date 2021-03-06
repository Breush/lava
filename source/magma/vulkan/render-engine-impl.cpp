#include "./render-engine-impl.hpp"

#include "../aft-vulkan/scene-aft.hpp"
#include "../shmag-reader.hpp"
#include "./helpers/queue.hpp"
#include "./holders/swapchain-holder.hpp"
#include "./render-image-impl.hpp"
#include "./render-targets/i-render-target-impl.hpp"
#include "./stages/present.hpp"

using namespace lava::magma;
using namespace lava::chamber;

RenderEngine::Impl::Impl(RenderEngine& engine)
    : m_engine(engine)
    , m_dummyImageHolder{*this, "magma.vulkan.render-engine.dummy-image"}
    , m_dummyNormalImageHolder{*this, "magma.vulkan.render-engine.dummy-normal-image"}
    , m_dummyInvisibleImageHolder{*this, "magma.vulkan.render-engine.dummy-invisible-image"}
    , m_dummyCubeImageHolder{*this, "magma.vulkan.render-engine.dummy-cube-image"}
{
    // @note This VR initialisation has to be done before initVulkan, because we need
    // to get the extensions list to be enable.
    initVr();
    initVulkan();
}

RenderEngine::Impl::~Impl()
{
    vr::VR_Shutdown();

    device().waitIdle();

    for (auto scene : m_scenes) {
        m_engine.sceneAllocator().deallocate(scene);
    }
}

void RenderEngine::Impl::update()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    updateVr();
    updateShaders();

    for (auto scene : m_scenes) {
        scene->aft().update();
    }

    // @note The difference between RenderTarget::Impl::update and RenderTarget::Impl::prepare
    // is that the latter should not update any UBOs.
    // For instance, VrRenderTarget update is updating the VrCameraController for the eyes,
    // and that should not be in prepare otherwise there might be concurrency issues with scene recording.
    for (auto renderTargetId = 0u; renderTargetId < m_renderTargetBundles.size(); ++renderTargetId) {
        auto& renderTargetBundle = m_renderTargetBundles[renderTargetId];
        auto& renderTargetImpl = renderTargetBundle.renderTarget->interfaceImpl();
        renderTargetImpl.update();
    }
}

void RenderEngine::Impl::draw()
{
    // @note This function does both render and draw, so no color.
    PROFILE_FUNCTION();

    // We record all render scenes cameras once
    for (auto scene : m_scenes) {
        scene->aft().record();
    }

    // Prepare all render targets
    for (auto renderTargetId = 0u; renderTargetId < m_renderTargetBundles.size(); ++renderTargetId) {
        auto& renderTargetBundle = m_renderTargetBundles[renderTargetId];
        auto& renderTargetImpl = renderTargetBundle.renderTarget->interfaceImpl();

        renderTargetBundle.prepareOk = renderTargetImpl.prepare();
    }

    // Wait for all scenes to be done with recording
    for (auto sceneId = 0u; sceneId < m_scenes.size(); ++sceneId) {
        auto scene = m_scenes[sceneId];
        scene->aft().waitRecord();

        // Tracking
        if (m_logTracking) {
            logger.info("magma.render-engine") << "Render scene " << sceneId << "." << std::endl;
            logger.log().tab(1);
            logger.log() << "draw-calls.flat-renderer: " << tracker.counter("draw-calls.flat-renderer") << std::endl;
            logger.log() << "draw-calls.renderer: " << tracker.counter("draw-calls.renderer") << std::endl;
            logger.log() << "draw-calls.shadows: " << tracker.counter("draw-calls.shadows") << std::endl;
            logger.log().tab(-1);
            m_logTracking = false;
        }
    }

    // Submit all the command buffers and present to the render targets
    for (auto renderTargetId = 0u; renderTargetId < m_renderTargetBundles.size(); ++renderTargetId) {
        auto& renderTargetBundle = m_renderTargetBundles[renderTargetId];

        if (!renderTargetBundle.prepareOk) continue;

        // Record command buffer each frame
        auto& renderTargetImpl = renderTargetBundle.renderTarget->interfaceImpl();
        auto currentIndex = renderTargetImpl.currentBufferIndex();
        const auto& commandBuffers = recordCommandBuffer(renderTargetId, currentIndex);
        renderTargetImpl.draw(commandBuffers);
    }
}

uint32_t RenderEngine::Impl::registerMaterialFromFile(const std::string& hrid, const fs::Path& shaderPath)
{
    PROFILE_FUNCTION(PROFILER_COLOR_REGISTER);

    if (m_materialInfos.find(hrid) != m_materialInfos.end()) {
        logger.warning("magma.vulkan.render-engine").tab(1) << "Material " << hrid << " has already been registered." << std::endl;
        return m_materialInfos[hrid].id;
    }

    auto materialId = m_materialInfos.size();

    logger.info("magma.vulkan.render-engine").tab(1) << "Registering material " << hrid << " as " << materialId << "." << std::endl;
    logger.log().tab(-1);

    // @note ShaderManager cannot handle shmag directly, it uses @magma:impl thingy
    // to be able to switch-case them or so in other renderer shaders.

    ShmagReader shmagReader(shaderPath);
    auto globalUniformDefinitions = shmagReader.globalUniformDefinitions();
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
    materialInfo.globalUniformDefinitions = globalUniformDefinitions;
    materialInfo.uniformDefinitions = uniformDefinitions;
    materialInfo.sourcePath = shaderPath;
    materialInfo.watchId = watchId;

    m_shadersManager.registerImplGroup(hrid, shaderImplementation, materialId);

    return materialId;
}

uint32_t RenderEngine::Impl::addView(RenderImage renderImage, IRenderTarget& renderTarget, const Viewport& viewport)
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
    // @fixme We should be able to forward the RenderImage directly
    auto imageView = renderImageImpl.view();
    auto imageLayout = renderImageImpl.layout();
    auto channelCount = renderImageImpl.channelCount();

    // @fixme Should be "compositorViewId"...
    renderView.presentViewId =
        renderTargetBundle.renderTarget->interfaceImpl().addView(imageView, imageLayout, m_dummySampler.get(), viewport, channelCount);

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

void RenderEngine::Impl::add(Scene& scene)
{
    const uint32_t sceneId = m_scenes.size();

    logger.info("magma.vulkan.render-engine") << "Adding render scene " << sceneId << "." << std::endl;
    logger.log().tab(1);

    // If no device yet, the scene initialization will be postponed
    // until it is created.
    if (m_deviceHolder.device()) {
        scene.aft().init();
    }

    m_scenes.emplace_back(&scene);

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
    if (!renderImageImpl.image()) return;

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
                                                                    m_dummySampler.get());
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

const MaterialInfo* RenderEngine::Impl::materialInfoIfExists(const std::string& hrid) const
{
    const auto iMaterialInfo = m_materialInfos.find(hrid);
    if (iMaterialInfo == m_materialInfos.end()) {
        return nullptr;
    }
    return &iMaterialInfo->second;
}

void RenderEngine::Impl::createCommandPools(vk::SurfaceKHR* pSurface)
{
    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

    logger.info("magma.vulkan.render-engine") << "Creating command pool." << std::endl;

    auto queueFamilyIndices = vulkan::findQueueFamilies(physicalDevice(), pSurface);

    vk::CommandPoolCreateInfo createInfo;
    createInfo.queueFamilyIndex = queueFamilyIndices.graphics;
    createInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    auto result = device().createCommandPoolUnique(createInfo);
    m_commandPool = vulkan::checkMove(result, "render-engine", "Unable to create command pool.");

    createInfo.queueFamilyIndex = queueFamilyIndices.transfer;
    createInfo.flags = vk::CommandPoolCreateFlagBits(0x0);

    result = device().createCommandPoolUnique(createInfo);
    m_transferCommandPool = vulkan::checkMove(result, "render-engine", "Unable to create transfer command pool.");
}

void RenderEngine::Impl::createDummyTextures()
{
    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

    logger.info("magma.vulkan.render-engine") << "Creating dummy textures." << std::endl;

    // Plain white
    std::vector<uint8_t> dummyData = {0xFF, 0xFF, 0xFF, 0xFF};
    m_dummyImageHolder.setup(dummyData, 1, 1, 4);

    // Flat blue
    dummyData = {0x80, 0x80, 0xFF, 0xFF}; // 0.5, 0.5, 1.0
    m_dummyNormalImageHolder.setup(dummyData, 1, 1, 4);

    // Full zeros
    dummyData = {0x00, 0x00, 0x00, 0x00};
    m_dummyInvisibleImageHolder.setup(dummyData, 1, 1, 4);

    // Plain transparent for cube maps
    std::vector<uint8_t> dummyCubeData = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    m_dummyCubeImageHolder.setup(dummyCubeData, 1, 1, 4, 6);

    // Sampler
    vk::SamplerCreateInfo createInfo;
    createInfo.magFilter = vk::Filter::eLinear;
    createInfo.minFilter = vk::Filter::eLinear;
    createInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    createInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    createInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    createInfo.anisotropyEnable = true;
    createInfo.maxAnisotropy = 16; // Over 16 is useless, but lower that for better performances
    createInfo.unnormalizedCoordinates = false;
    createInfo.compareEnable = false;
    createInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    createInfo.maxLod = 10;

    auto result = device().createSamplerUnique(createInfo);
    m_dummySampler = vulkan::checkMove(result, "render-engine", "Unable to create dummy sampler.");

    // Shadows sampler
    createInfo.magFilter = vk::Filter::eLinear;
    createInfo.minFilter = vk::Filter::eLinear;
    createInfo.addressModeU = vk::SamplerAddressMode::eClampToBorder;
    createInfo.addressModeV = vk::SamplerAddressMode::eClampToBorder;
    createInfo.addressModeW = vk::SamplerAddressMode::eClampToBorder;
    createInfo.anisotropyEnable = false;
    createInfo.unnormalizedCoordinates = false;
    createInfo.compareEnable = false;
    createInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;

    result = device().createSamplerUnique(createInfo);
    m_shadowsSampler = vulkan::checkMove(result, "render-engine", "Unable to create shadows sampler.");
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

    for (auto scene : m_scenes) {
        const auto& sceneCommandBuffers = scene->aft().commandBuffers();
        commandBuffers.insert(commandBuffers.end(), sceneCommandBuffers.begin(), sceneCommandBuffers.end());
    }

    //----- Render targets' specific rendering

    auto commandBuffer = renderTargetBundle.commandBuffers[bufferIndex].get();

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

    // (Re-)allocate them all
    commandBuffers.resize(renderTargetBundle.renderTarget->interfaceImpl().buffersCount());

    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.commandPool = m_commandPool.get();
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = commandBuffers.size();

    auto result = device().allocateCommandBuffersUnique(allocInfo);
    commandBuffers = vulkan::checkMove(result, "render-engine", "Unable to create command buffers.");
}

void RenderEngine::Impl::initScenes()
{
    if (m_scenes.size() == 0u) return;

    PROFILE_FUNCTION(PROFILER_COLOR_INIT);

    logger.info("magma.vulkan.render-engine") << "Initializing render scenes." << std::endl;
    logger.log().tab(1);

    for (const auto& scene : m_scenes) {
        scene->aft().init();
    }

    logger.log().tab(-1);
}

void RenderEngine::Impl::initVr()
{
    PROFILE_FUNCTION(PROFILER_COLOR_INIT);

    logger.info("magma.vulkan.render-engine") << "Initializing VR." << std::endl;
    logger.log().tab(1);

    // @todo vr().init() / vr().update() should probably be called by RenderEngine itself (not Impl)
    // as this is not vulkan dependent
    m_engine.vr().init();

    logger.log().tab(-1);
}

void RenderEngine::Impl::initVulkan()
{
    PROFILE_FUNCTION(PROFILER_COLOR_INIT);

    logger.info("magma.vulkan.render-engine") << "Initializing vulkan." << std::endl;
    logger.log().tab(1);

    m_instanceHolder.init(true, m_engine.vr());

    logger.log().tab(-1);
}

void RenderEngine::Impl::initVulkanDevice(vk::SurfaceKHR* pSurface)
{
    PROFILE_FUNCTION(PROFILER_COLOR_INIT);

    logger.info("magma.vulkan.render-engine") << "Initializing vulkan device." << std::endl;
    logger.log().tab(1);

    m_deviceHolder.init(instance(), pSurface, m_instanceHolder.debugEnabled(), m_engine.vr());

    createCommandPools(pSurface);
    createDummyTextures();

    initScenes();

    logger.log().tab(-1);
}

void RenderEngine::Impl::updateVr()
{
    if (m_engine.vr().enabled()) {
        m_engine.vr().update();
    }
}

void RenderEngine::Impl::updateShaders()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    while (auto event = m_shadersWatcher.pollEvent()) {
        if (event->type == chamber::FileWatchEvent::Type::Modified) {
            auto materialPath = event->path;
            auto watchId = event->watchId;

            logger.info("magma.vulkan.render-engine") << "Material source file " << event->path << " has changed." << std::endl;
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

    m_shadersManager.update();
}
