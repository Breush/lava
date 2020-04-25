#include "./scene-aft.hpp"

#include <lava/magma/camera.hpp>
#include <lava/magma/flat.hpp>
#include <lava/magma/light.hpp>
#include <lava/magma/material.hpp>
#include <lava/magma/mesh.hpp>
#include <lava/magma/scene.hpp>

#include "../vulkan/render-engine-impl.hpp"
#include "../vulkan/stages/deep-deferred-stage.hpp"
#include "../vulkan/stages/forward-flat-stage.hpp"
#include "../vulkan/stages/forward-renderer-stage.hpp"
#include "../vulkan/stages/shadows-stage.hpp"
#include "./camera-aft.hpp"
#include "./config.hpp"
#include "./flat-aft.hpp"
#include "./light-aft.hpp"
#include "./material-aft.hpp"
#include "./mesh-aft.hpp"

using namespace lava::chamber;
using namespace lava::magma;

namespace {
    static uint16_t g_lightId = 0;
    static uint16_t g_cameraId = 0;
}

SceneAft::SceneAft(Scene& scene, RenderEngine& engine)
    : m_fore(scene)
    , m_engine(engine)
    , m_lightsDescriptorHolder(engine.impl())
    , m_shadowsDescriptorHolder(engine.impl())
    , m_materialDescriptorHolder(engine.impl())
    , m_materialGlobalDescriptorHolder(engine.impl())
    , m_environmentDescriptorHolder(engine.impl())
    , m_environment(scene, engine)
{
}

void SceneAft::init()
{
    m_initialized = true;

    m_lightsDescriptorHolder.uniformBufferSizes({1});
    m_lightsDescriptorHolder.init(64, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

    m_shadowsDescriptorHolder.uniformBufferSizes({1});
    m_shadowsDescriptorHolder.combinedImageSamplerSizes({SHADOWS_CASCADES_COUNT});
    m_shadowsDescriptorHolder.init(256, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

    m_materialDescriptorHolder.uniformBufferSizes({1});
    m_materialDescriptorHolder.combinedImageSamplerSizes({MATERIAL_SAMPLERS_SIZE, 1}); // samplers, cubeSampler
    m_materialDescriptorHolder.init(1024, vk::ShaderStageFlagBits::eFragment);

    m_materialGlobalDescriptorHolder.combinedImageSamplerSizes({MATERIAL_SAMPLERS_SIZE});
    m_materialGlobalDescriptorHolder.init(1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

    m_materialGlobalDescriptorSet = m_materialGlobalDescriptorHolder.allocateSet("engine.material-global");
    auto& engine = m_engine.impl();
    const auto imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    for (auto i = 0u; i < MATERIAL_SAMPLERS_SIZE; ++i) {
        vulkan::updateDescriptorSet(engine.device(), m_materialGlobalDescriptorSet, engine.dummyImageView(),
                                    engine.dummySampler(), imageLayout, 0u, i);
    }

    // environmentRadianceMap, environmentIrradianceMap, brdfLut
    m_environmentDescriptorHolder.combinedImageSamplerSizes({1, 1, 1});
    m_environmentDescriptorHolder.init(2, vk::ShaderStageFlagBits::eFragment);

    initStages();
    initResources();
}

void SceneAft::update()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    m_frameId = (m_frameId + 1u) % FRAME_IDS_COUNT;

    if (!m_pendingRemovedMeshes.empty()) {
        // @note This is necessary because we are no waiting for device on each update.
        m_engine.impl().device().waitIdle();

        // @note We need to store the count of removed mesh here,
        // because some others might be added during the removal.
        auto removedMeshCount = m_pendingRemovedMeshes.size();
        for (auto i = 0u; i < removedMeshCount; ++i) {
            m_fore.removeUnsafe(*m_pendingRemovedMeshes[i]);
        }
        m_pendingRemovedMeshes.erase(m_pendingRemovedMeshes.begin(), m_pendingRemovedMeshes.begin() + removedMeshCount);
    }

    // @todo Some light or cameras might be inactive,
    // we should add this concept.
    // We should also be sure this is not done too many times.
    for (auto light : m_fore.lights()) {
        light->aft().update();

        for (auto& shadows : m_lightBundles[light].shadows) {
            if (m_cameraBundles.at(shadows.first).shadowsFallbackCamera != nullptr) continue;
            shadows.second.update(m_frameId);
        }
    }

    for (auto material : m_fore.materials()) {
        material->aft().update();
    }

    for (auto mesh : m_fore.meshes()) {
        mesh->aft().update();
    }

    for (auto flat : m_fore.flats()) {
        flat->aft().update();
    }
}

// ----- Record

void SceneAft::record()
{
    tracker.counter("draw-calls.shadows") = 0u;
    tracker.counter("draw-calls.renderer") = 0u;
    tracker.counter("draw-calls.flat-renderer") = 0u;

    m_commandBuffers.resize(0);

    // @note :ShadowsLightCameraPair The order here is important, because it is how everything
    // is going to be rendered. So the pre-pass of constructing shadow maps
    // based on the camera has to be done first.
    // They all use the very same shadows stage to update,
    // but that does not matter as long as the command buffers within the threads
    // are all different and as long as the shadows stage does not keep local state for
    // rendering.

    // @todo Don't have notion of "active" cameras yet, so we update them all
    for (auto camera : m_fore.cameras()) {
        const auto& cameraBundle = m_cameraBundles[camera];
        if (cameraBundle.shadowsFallbackCamera == nullptr) {
            for (auto light : m_fore.lights()) {
                auto& lightBundle = m_lightBundles[light];
                if (light->shadowsEnabled()) {
                    auto& shadowsThread = lightBundle.shadowsThreads.at(camera);
                    shadowsThread.record(*lightBundle.shadowsStage, camera);
                    m_commandBuffers.emplace_back(shadowsThread.commandBuffer());
                }
            }
        }

        cameraBundle.rendererThread->record(*cameraBundle.rendererStage, m_frameId);
        m_commandBuffers.emplace_back(cameraBundle.rendererThread->commandBuffer());
    }
}

void SceneAft::waitRecord()
{
    for (auto camera : m_fore.cameras()) {
        for (auto light : m_fore.lights()) {
            auto& lightBundle = m_lightBundles[light];
            if (light->shadowsEnabled()) {
                lightBundle.shadowsThreads.at(camera).wait();
            }
        }

        const auto& cameraBundle = m_cameraBundles[camera];
        cameraBundle.rendererThread->wait();
    }
}

// ----- Cameras API

RenderImage SceneAft::cameraRenderImage(const Camera& camera) const
{
    return m_cameraBundles.at(&camera).rendererStage->renderImage();
}

RenderImage SceneAft::cameraDepthRenderImage(const Camera& camera) const
{
    return m_cameraBundles.at(&camera).rendererStage->depthRenderImage();
}

void SceneAft::updateCamera(const Camera& camera)
{
    rebuildStages(camera);

    auto& rendererStage = *m_cameraBundles.at(&camera).rendererStage;

    // Present stage of RenderEngine is not right anymore, has the renderImage is no more
    m_engine.impl().updateRenderViews(rendererStage.renderImage());

    // @note Well, the shadow map might not have changed,
    // but we update it anyway. We're not sure if it is the first initialization or not.
    if (rendererStage.depthRenderImageValid()) {
        m_engine.impl().updateRenderViews(rendererStage.depthRenderImage());
    }
}

void SceneAft::changeCameraRenderImageLayout(const Camera& camera, vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer)
{
    m_cameraBundles.at(&camera).rendererStage->changeRenderImageLayout(imageLayout, commandBuffer);
}

// ----- Shadows

const Shadows& SceneAft::shadows(const Light& light, const Camera& camera) const
{
    auto pCamera = &camera;

    auto& cameraBundle = m_cameraBundles.at(pCamera);
    if (cameraBundle.shadowsFallbackCamera != nullptr) {
        pCamera = cameraBundle.shadowsFallbackCamera;
    }

    return m_lightBundles.at(&light).shadows.at(pCamera);
}

void SceneAft::shadowsFallbackCamera(Camera& camera, const Camera& fallbackCamera)
{
    m_cameraBundles.at(&camera).shadowsFallbackCamera = &fallbackCamera;
}

RenderImage SceneAft::shadowsCascadeRenderImage(const Light& light, const Camera* camera, uint32_t cascadeIndex) const
{
    // No camera specified? Just take the first one.
    if (camera == nullptr) {
        camera = m_fore.cameras()[0u];
    }

    auto& cameraBundle = m_cameraBundles.at(camera);
    if (cameraBundle.shadowsFallbackCamera != nullptr) {
        camera = cameraBundle.shadowsFallbackCamera;
    }

    return m_lightBundles.at(&light).shadowsStage->renderImage(*camera, cascadeIndex);
}

float SceneAft::shadowsCascadeSplitDepth(const Light& light, const Camera& camera, uint32_t cascadeIndex) const
{
    return m_lightBundles.at(&light).shadows.at(&camera).cascadeSplitDepth(cascadeIndex);
}

const glm::mat4& SceneAft::shadowsCascadeTransform(const Light& light, const Camera& camera, uint32_t cascadeIndex) const
{
    return m_lightBundles.at(&light).shadows.at(&camera).cascadeTransform(cascadeIndex);
}

// ----- Fore

void SceneAft::foreAdd(Light& light)
{
    const uint16_t lightId = g_lightId++;

    logger.info("magma.vulkan.scene") << "Adding light " << lightId << "." << std::endl;
    logger.log().tab(1);

    auto& lightBundle = m_lightBundles[&light];
    lightBundle.id = lightId;

    // Setting shadows
    lightBundle.shadowsStage = std::make_unique<ShadowsStage>(m_fore);

    if (m_initialized) {
        light.aft().init();
        lightBundle.shadowsStage->init(light);
        lightBundle.shadowsStage->update({SHADOW_MAP_SIZE, SHADOW_MAP_SIZE});
    }

    updateLightBundleFromCameras(light);

    logger.log().tab(-1);
}

void SceneAft::foreAdd(Camera& camera)
{
    const uint16_t cameraId = g_cameraId++;

    logger.info("magma.vulkan.scene") << "Adding camera " << cameraId << " (" << m_fore.rendererType() << ")." << std::endl;
    logger.log().tab(1);

    auto& cameraBundle = m_cameraBundles[&camera];
    cameraBundle.id = cameraId;
    cameraBundle.rendererThread = std::make_unique<vulkan::CommandBufferThread>(m_engine.impl(), "camera.renderer");

    if (m_fore.rendererType() == RendererType::DeepDeferred) {
        cameraBundle.rendererStage = std::make_unique<DeepDeferredStage>(m_fore);
    }
    else if (m_fore.rendererType() == RendererType::ForwardFlat) {
        cameraBundle.rendererStage = std::make_unique<ForwardFlatStage>(m_fore);
    }
    else {
        // @note We fallback to a forward renderer if something is not handled.
        cameraBundle.rendererStage = std::make_unique<ForwardRendererStage>(m_fore);
    }

    cameraBundle.rendererStage->sampleCount(sampleCount());

    if (m_initialized) {
        cameraBundle.rendererStage->init(camera);
        rebuildStages(camera);
    }

    // :ShadowsLightCameraPair We neeed to resize the number of threads for shadow map generation
    for (auto light : m_fore.lights()) {
        auto& lightBundle = m_lightBundles[light];
        lightBundle.shadowsStage->updateFromCamerasCount();
        updateLightBundleFromCameras(*light);
    }

    logger.log().tab(-1);
}

void SceneAft::foreAdd(Material& material)
{
    if (m_initialized) {
        material.aft().init();
    }
}

void SceneAft::foreRemove(const Light& light)
{
    m_engine.impl().device().waitIdle();
    m_fore.removeUnsafe(light);
}

void SceneAft::foreRemove(const Camera& camera)
{
    m_engine.impl().device().waitIdle();
    m_fore.removeUnsafe(camera);
}

void SceneAft::foreRemove(const Mesh& mesh)
{
    m_pendingRemovedMeshes.emplace_back(&mesh);
}

void SceneAft::foreRemove(const Flat& flat)
{
    m_engine.impl().device().waitIdle();
    m_fore.removeUnsafe(flat);
}

void SceneAft::foreMsaaChanged()
{
    for (auto& camera : m_fore.cameras()) {
        updateCamera(*camera);
    }
}

// ----- Internal

void SceneAft::initStages()
{
    logger.info("magma.vulkan.scene") << "Initializing render stages." << std::endl;
    logger.log().tab(1);

    for (auto camera : m_fore.cameras()) {
        m_cameraBundles[camera].rendererStage->init(*camera);
        updateCamera(*camera);
    }

    for (auto light : m_fore.lights()) {
        auto& lightBundle = m_lightBundles[light];
        light->aft().init();
        lightBundle.shadowsStage->init(*light);
        lightBundle.shadowsStage->update({SHADOW_MAP_SIZE, SHADOW_MAP_SIZE});
        for (auto camera : m_fore.cameras()) {
            lightBundle.shadows.at(camera).init(*light, *camera);
        }
    }

    logger.log().tab(-1);
}

void SceneAft::initResources()
{
    for (auto light : m_fore.lights()) {
        light->aft().init();
    }

    for (auto& material : m_fore.materials()) {
        material->aft().init();
    }

    m_environment.init();
}

void SceneAft::rebuildStages(const Camera& camera)
{
    auto& rendererStage = *m_cameraBundles.at(&camera).rendererStage;

    // Extent update
    const auto& extent = camera.extent();
    vk::Extent2D vkExtent{extent.width, extent.height};
    const auto polygonMode = camera.polygonMode();
    rendererStage.extent(vkExtent);
    rendererStage.sampleCount(sampleCount());
    rendererStage.polygonMode((polygonMode == PolygonMode::Line) ? vk::PolygonMode::eLine : vk::PolygonMode::eFill);
    rendererStage.rebuild();
}

void SceneAft::updateLightBundleFromCameras(const Light& light)
{
    auto& lightBundle = m_lightBundles.at(&light);
    for (auto camera : m_fore.cameras()) {
        if (lightBundle.shadows.find(camera) != lightBundle.shadows.end()) continue;

        // Create all shadows stages and threads that do not exist yet
        lightBundle.shadowsThreads.emplace(std::piecewise_construct, std::forward_as_tuple(camera),
                                           std::forward_as_tuple(m_engine.impl(), "light.shadows"));
        lightBundle.shadows.emplace(camera, m_fore);

        if (m_initialized) {
            lightBundle.shadows.at(camera).init(light, *camera);
        }
    }
}

vk::SampleCountFlagBits SceneAft::sampleCount() const
{
    if (m_fore.msaa() == Msaa::Max) {
        return m_engine.impl().deviceHolder().maxSampleCount();
    }

    return vk::SampleCountFlagBits::e1;
}
