#include "./render-scene-impl.hpp"

#include "../cameras/i-camera-impl.hpp"
#include "../lights/i-light-impl.hpp"
#include "../material-impl.hpp"
#include "../mesh-impl.hpp"
#include "../render-engine-impl.hpp"
#include "../stages/deep-deferred-stage.hpp"
#include "../stages/forward-renderer-stage.hpp"
#include "../stages/shadows-stage.hpp"
#include "../texture-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

RenderScene::Impl::Impl(RenderEngine& engine, RenderScene& scene)
    : m_scene(scene)
    , m_engine(engine.impl())
    , m_rendererType(RendererType::Forward)
    , m_lightsDescriptorHolder(m_engine)
    , m_shadowsDescriptorHolder(m_engine)
    , m_materialDescriptorHolder(m_engine)
    , m_environmentDescriptorHolder(m_engine)
    , m_environment(*this)
{
}

RenderScene::Impl::~Impl() = default;

//----- IRenderScene

void RenderScene::Impl::init(uint32_t id)
{
    m_id = id;
    m_initialized = true;

    m_lightsDescriptorHolder.uniformBufferSizes({1});
    m_lightsDescriptorHolder.init(64, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

    m_shadowsDescriptorHolder.uniformBufferSizes({1});
    m_shadowsDescriptorHolder.combinedImageSamplerSizes({SHADOWS_CASCADES_COUNT});
    m_shadowsDescriptorHolder.init(256, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

    m_materialDescriptorHolder.uniformBufferSizes({1});
    m_materialDescriptorHolder.combinedImageSamplerSizes({8, 1}); // 8 samplers, 1 cubeSampler
    m_materialDescriptorHolder.init(128, vk::ShaderStageFlagBits::eFragment);

    // environmentRadianceMap, environmentIrradianceMap, brdfLut
    m_environmentDescriptorHolder.combinedImageSamplerSizes({1, 1, 1});
    m_environmentDescriptorHolder.init(2, vk::ShaderStageFlagBits::eFragment);

    initStages();
    initResources();
}

void RenderScene::Impl::update()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    if (!m_pendingRemovedMeshes.empty()) {
        // @note This is necessary because we are no waiting for device on each update.
        m_engine.device().waitIdle();

        // @note We need to copy list of pending removed meshes
        // because it can be updated while removing.
        auto pendingRemovedMeshes = m_pendingRemovedMeshes;

        for (auto mesh : pendingRemovedMeshes) {
            for (auto iMeshImpl = m_meshesImpls.begin(); iMeshImpl != m_meshesImpls.end(); ++iMeshImpl) {
                if (*iMeshImpl == &mesh->impl()) {
                    m_meshesImpls.erase(iMeshImpl);
                    break;
                }
            }

            for (auto iMesh = m_meshes.begin(); iMesh != m_meshes.end(); ++iMesh) {
                if (iMesh->get() == mesh) {
                    m_meshes.erase(iMesh);
                    break;
                }
            }
        }
    }

    // @todo Some light or cameras might be inactive,
    // we should add this concept.
    // We should also be sure this is not done to many times.
    for (auto& lightBundle : m_lightBundles) {
        for (auto& shadows : lightBundle.shadows) {
            shadows->update();
        }
    }
}

void RenderScene::Impl::record()
{
    tracker.counter("draw-calls.shadows") = 0u;
    tracker.counter("draw-calls.renderer") = 0u;

    m_commandBuffers.resize(0);

    // @note :ShadowsLightCameraPair The order here is important, because it is how everything
    // is going to be rendered. So the pre-pass of constructing shadow maps
    // based on the camera has to be done first.
    // They all use the very same shadows stage to update,
    // but that does not matter as long as the command buffers within the threads
    // are all different and as long as the shadows stage does not keep local state for
    // rendering.

    // @todo Don't have notion of "active" cameras yet, so we update them all
    for (auto cameraId = 0u; cameraId < m_cameraBundles.size(); ++cameraId) {
        for (auto& lightBundle : m_lightBundles) {
            if (lightBundle.light->shadowsEnabled()) {
                lightBundle.shadowsThreads[cameraId]->record(*lightBundle.shadowsStage, cameraId);
                m_commandBuffers.emplace_back(lightBundle.shadowsThreads[cameraId]->commandBuffer());
            }
        }

        const auto& cameraBundle = m_cameraBundles[cameraId];
        cameraBundle.rendererThread->record(*cameraBundle.rendererStage);
        m_commandBuffers.emplace_back(cameraBundle.rendererThread->commandBuffer());
    }
}

void RenderScene::Impl::waitRecord()
{
    for (auto cameraId = 0u; cameraId < m_cameraBundles.size(); ++cameraId) {
        for (auto& lightBundle : m_lightBundles) {
            if (lightBundle.light->shadowsEnabled()) {
                lightBundle.shadowsThreads[cameraId]->wait();
            }
        }

        const auto& cameraBundle = m_cameraBundles[cameraId];
        cameraBundle.rendererThread->wait();
    }
}

void RenderScene::Impl::add(std::unique_ptr<ICamera>&& camera)
{
    const uint32_t cameraId = m_cameraBundles.size();

    logger.info("magma.vulkan.render-scenes.render-scene")
        << "Adding camera " << cameraId << " (" << m_rendererType << ")." << std::endl;
    logger.log().tab(1);

    m_cameraBundles.emplace_back();
    auto& cameraBundle = m_cameraBundles.back();
    cameraBundle.camera = std::move(camera);
    cameraBundle.rendererThread = std::make_unique<vulkan::CommandBufferThread>(m_engine, "camera.renderer");

    // @note We fallback to a forward renderer if something is not handled.
    if (m_rendererType == RendererType::DeepDeferred)
        cameraBundle.rendererStage = std::make_unique<DeepDeferredStage>(*this);
    else
        cameraBundle.rendererStage = std::make_unique<ForwardRendererStage>(*this);

    if (m_initialized) {
        cameraBundle.camera->interfaceImpl().init(cameraId);
        cameraBundle.rendererStage->init(cameraId);
        updateStages(cameraId);
    }

    // :ShadowsLightCameraPair We neeed to resize the number of threads for shadow map generation
    for (auto lightId = 0u; lightId < m_lightBundles.size(); ++lightId) {
        auto& lightBundle = m_lightBundles[lightId];
        lightBundle.shadowsStage->updateFromCamerasCount();
        lightBundle.shadows.resize(m_cameraBundles.size());
        lightBundle.shadowsThreads.resize(m_cameraBundles.size());
        for (auto cameraId = 0u; cameraId < m_cameraBundles.size(); ++cameraId) {
            if (lightBundle.shadows[cameraId] == nullptr) {
                lightBundle.shadows[cameraId] = std::make_unique<Shadows>(*this);
                lightBundle.shadows[cameraId]->init(lightId, cameraId);
            }
            if (lightBundle.shadowsThreads[cameraId] == nullptr) {
                lightBundle.shadowsThreads[cameraId] = std::make_unique<vulkan::CommandBufferThread>(m_engine, "light.shadows");
            }
        }
    }

    logger.log().tab(-1);
}

void RenderScene::Impl::add(std::unique_ptr<Material>&& material)
{
    if (m_initialized) {
        material->impl().init();
    }

    m_materials.emplace_back(std::move(material));
}

void RenderScene::Impl::add(std::unique_ptr<Texture>&& texture)
{
    if (m_initialized) {
        texture->impl().init();
    }

    m_textures.emplace_back(std::move(texture));
}

void RenderScene::Impl::add(std::unique_ptr<Mesh>&& mesh)
{
    if (m_initialized) {
        mesh->impl().init();
    }

    m_meshes.emplace_back(std::move(mesh));
    m_meshesImpls.emplace_back(&m_meshes.back()->impl());
}

void RenderScene::Impl::add(std::unique_ptr<ILight>&& light)
{
    const uint32_t lightId = m_lightBundles.size();

    logger.info("magma.vulkan.render-scenes.render-scene") << "Adding light " << lightId << "." << std::endl;
    logger.log().tab(1);

    m_lightBundles.emplace_back();
    auto& lightBundle = m_lightBundles.back();
    lightBundle.light = std::move(light);

    // Setting shadows
    if (lightBundle.light->shadowsEnabled()) {
        lightBundle.shadowsStage = std::make_unique<ShadowsStage>(*this);
        lightBundle.shadows.resize(m_cameraBundles.size());
        lightBundle.shadowsThreads.resize(m_cameraBundles.size());
        for (auto cameraId = 0u; cameraId < m_cameraBundles.size(); ++cameraId) {
            lightBundle.shadows[cameraId] = std::make_unique<Shadows>(*this);
            lightBundle.shadowsThreads[cameraId] = std::make_unique<vulkan::CommandBufferThread>(m_engine, "light.shadows");
        }
    }

    if (m_initialized) {
        lightBundle.light->interfaceImpl().init(lightId);
        if (lightBundle.light->shadowsEnabled()) {
            lightBundle.shadowsStage->init(lightId);
            lightBundle.shadowsStage->update(vk::Extent2D{SHADOW_MAP_SIZE, SHADOW_MAP_SIZE});
            for (auto cameraId = 0u; cameraId < m_cameraBundles.size(); ++cameraId) {
                lightBundle.shadows[cameraId]->init(lightId, cameraId);
            }
        }
    }

    logger.log().tab(-1);
}

void RenderScene::Impl::remove(const Mesh& mesh)
{
    // @note Some meshes might remove others within their destructor,
    // so this pending removed mesh list is necessary.

    m_pendingRemovedMeshes.emplace_back(&mesh);
}

void RenderScene::Impl::remove(const Material& material)
{
    m_engine.device().waitIdle();
    for (auto iMaterial = m_materials.begin(); iMaterial != m_materials.end(); ++iMaterial) {
        if (iMaterial->get() == &material) {
            m_materials.erase(iMaterial);
            break;
        }
    }
}

void RenderScene::Impl::remove(const Texture& texture)
{
    m_engine.device().waitIdle();
    for (auto iTexture = m_textures.begin(); iTexture != m_textures.end(); ++iTexture) {
        if (iTexture->get() == &texture) {
            m_textures.erase(iTexture);
            break;
        }
    }
}

//---- Internal interface

void RenderScene::Impl::updateCamera(uint32_t cameraId)
{
    updateStages(cameraId);

    const auto& camera = *m_cameraBundles[cameraId].camera;

    // Present stage of RenderEngine is not right anymore, has the renderImage is no more
    m_engine.updateRenderViews(camera.renderImage());

    // @note Well, the shadow map might not have changed,
    // but we update it anyway. We're not sure if it is the first initialization or not.
    m_engine.updateRenderViews(camera.depthRenderImage());
}

void RenderScene::Impl::fallbackMaterial(std::unique_ptr<Material>&& material)
{
    m_fallbackMaterial = std::move(material);

    if (m_initialized) {
        m_fallbackMaterial->impl().init();
    }
}

RenderImage RenderScene::Impl::cameraRenderImage(uint32_t cameraIndex) const
{
    if (!m_initialized) {
        logger.warning("magma.vulkan.render-scenes.render-scene")
            << "Trying to access camera depth render image " << cameraIndex << " but the scene is not initialized." << std::endl;
        return RenderImage();
    }

    if (cameraIndex >= m_cameraBundles.size()) {
        logger.warning("magma.vulkan.render-scenes.render-scene")
            << "Trying to access camera render image " << cameraIndex << " but only " << m_cameraBundles.size() << " added."
            << std::endl;
        return RenderImage();
    }

    return m_cameraBundles[cameraIndex].rendererStage->renderImage();
}

RenderImage RenderScene::Impl::cameraDepthRenderImage(uint32_t cameraIndex) const
{
    if (!m_initialized) {
        logger.warning("magma.vulkan.render-scenes.render-scene")
            << "Trying to access camera depth render image " << cameraIndex << " but the scene is not initialized." << std::endl;
        return RenderImage();
    }

    if (cameraIndex >= m_cameraBundles.size()) {
        logger.warning("magma.vulkan.render-scenes.render-scene")
            << "Trying to access camera depth render image " << cameraIndex << " but only " << m_cameraBundles.size() << " added."
            << std::endl;
        return RenderImage();
    }

    return m_cameraBundles[cameraIndex].rendererStage->depthRenderImage();
}

RenderImage RenderScene::Impl::shadowsCascadeRenderImage(uint32_t lightIndex, uint32_t cameraId, uint32_t cascadeIndex) const
{
    if (!m_initialized) {
        logger.warning("magma.vulkan.render-scenes.render-scene")
            << "Trying to access light shadows render image " << lightIndex << " but the scene is not initialized." << std::endl;
        return RenderImage();
    }

    if (lightIndex >= m_lightBundles.size()) {
        logger.warning("magma.vulkan.render-scenes.render-scene")
            << "Trying to access light shadows render image " << lightIndex << " but only " << m_lightBundles.size() << " added."
            << std::endl;
        return RenderImage();
    }

    return m_lightBundles[lightIndex].shadowsStage->renderImage(cameraId, cascadeIndex);
}

float RenderScene::Impl::shadowsCascadeSplitDepth(uint32_t lightIndex, uint32_t cameraIndex, uint32_t cascadeIndex) const
{
    return m_lightBundles[lightIndex].shadows[cameraIndex]->cascadeSplitDepth(cascadeIndex);
}

const glm::mat4& RenderScene::Impl::shadowsCascadeTransform(uint32_t lightIndex, uint32_t cameraIndex,
                                                            uint32_t cascadeIndex) const
{
    return m_lightBundles[lightIndex].shadows[cameraIndex]->cascadeTransform(cascadeIndex);
}

void RenderScene::Impl::changeCameraRenderImageLayout(uint32_t cameraIndex, vk::ImageLayout imageLayout,
                                                      vk::CommandBuffer commandBuffer)
{
    m_cameraBundles[cameraIndex].rendererStage->changeRenderImageLayout(imageLayout, commandBuffer);
}

//---- Internal

void RenderScene::Impl::initStages()
{
    logger.info("magma.vulkan.render-scenes.render-scene") << "Initializing render stages." << std::endl;
    logger.log().tab(1);

    for (auto cameraId = 0u; cameraId < m_cameraBundles.size(); ++cameraId) {
        auto& cameraBundle = m_cameraBundles[cameraId];
        cameraBundle.rendererStage->init(cameraId);
        updateCamera(cameraId);
    }

    for (auto lightId = 0u; lightId < m_cameraBundles.size(); ++lightId) {
        auto& lightBundle = m_lightBundles[lightId];
        lightBundle.light->interfaceImpl().init(lightId);
        if (lightBundle.light->shadowsEnabled()) {
            lightBundle.shadowsStage->init(lightId);
            lightBundle.shadowsStage->update(vk::Extent2D{1024u, 1024u});
            for (auto cameraId = 0u; cameraId < m_cameraBundles.size(); ++cameraId) {
                lightBundle.shadows[cameraId]->init(lightId, cameraId);
            }
        }
    }

    logger.log().tab(-1);
}

void RenderScene::Impl::initResources()
{
    for (auto cameraId = 0u; cameraId < m_cameraBundles.size(); ++cameraId) {
        auto& cameraBundle = m_cameraBundles[cameraId];
        cameraBundle.camera->interfaceImpl().init(cameraId);
    }

    if (m_fallbackMaterial != nullptr) {
        m_fallbackMaterial->impl().init();
    }

    for (auto& material : m_materials) {
        material->impl().init();
    }

    for (auto& mesh : m_meshes) {
        mesh->impl().init();
    }

    for (auto lightId = 0u; lightId < m_lightBundles.size(); ++lightId) {
        auto& lightBundle = m_lightBundles[lightId];
        lightBundle.light->interfaceImpl().init(lightId);
    }

    m_environment.init();
}

void RenderScene::Impl::updateStages(uint32_t cameraId)
{
    auto& cameraBundle = m_cameraBundles[cameraId];
    auto& rendererStage = *cameraBundle.rendererStage;

    // Extent update
    const auto& extent = cameraBundle.camera->interfaceImpl().renderExtent();
    const auto polygonMode = cameraBundle.camera->polygonMode();
    rendererStage.update(extent, (polygonMode == PolygonMode::Line) ? vk::PolygonMode::eLine : vk::PolygonMode::eFill);
}
