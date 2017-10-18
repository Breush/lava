#include "./render-scene-impl.hpp"

#include <lava/chamber/logger.hpp>

#include "../cameras/i-camera-impl.hpp"
#include "../lights/i-light-impl.hpp"
#include "../material-impl.hpp"
#include "../meshes/i-mesh-impl.hpp"
#include "../render-engine-impl.hpp"
#include "../stages/deep-deferred-stage.hpp"

using namespace lava::magma;
using namespace lava::chamber;

RenderScene::Impl::Impl(RenderEngine& engine)
    : m_engine(engine.impl())
    , m_cameraDescriptorHolder(m_engine)
    , m_materialDescriptorHolder(m_engine)
    , m_meshDescriptorHolder(m_engine)
{
}

RenderScene::Impl::~Impl() = default;

//----- IRenderScene

void RenderScene::Impl::init(uint32_t id)
{
    m_id = id;
    m_initialized = true;

    // Deep deferred renderer common descriptors
    m_cameraDescriptorHolder.uniformBufferSizes({1});
    m_cameraDescriptorHolder.init(16, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

    m_materialDescriptorHolder.uniformBufferSizes({1});
    m_materialDescriptorHolder.combinedImageSamplerSizes({1, 1, 1});
    m_materialDescriptorHolder.init(128, vk::ShaderStageFlagBits::eFragment);

    m_meshDescriptorHolder.uniformBufferSizes({1});
    m_meshDescriptorHolder.init(128, vk::ShaderStageFlagBits::eVertex);

    initStages();
    initResources();
}

void RenderScene::Impl::render(vk::CommandBuffer commandBuffer)
{
    for (const auto& cameraBundle : m_cameraBundles) {
        cameraBundle.deepDeferredStage->render(commandBuffer);
    }
}

void RenderScene::Impl::add(std::unique_ptr<ICamera>&& camera)
{
    const uint32_t cameraId = m_cameraBundles.size();

    logger.info("magma.vulkan.render-scenes.render-scene") << "Adding camera " << cameraId << "." << std::endl;
    logger.log().tab(1);

    m_cameraBundles.emplace_back();
    auto& cameraBundle = m_cameraBundles.back();
    cameraBundle.camera = std::move(camera);
    cameraBundle.deepDeferredStage = std::make_unique<DeepDeferredStage>(*this);

    if (m_initialized) {
        cameraBundle.camera->interfaceImpl().init(cameraId);
        cameraBundle.deepDeferredStage->init(cameraId);
        updateStages(cameraId);
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

void RenderScene::Impl::add(std::unique_ptr<IMesh>&& mesh)
{
    if (m_initialized) {
        mesh->interfaceImpl().init();
    }

    m_meshes.emplace_back(std::move(mesh));
}

void RenderScene::Impl::add(std::unique_ptr<ILight>&& light)
{
    if (m_initialized) {
        light->interfaceImpl().init();
    }

    m_lights.emplace_back(std::move(light));
}

void RenderScene::Impl::remove(const IMesh& mesh)
{
    m_engine.device().waitIdle();
    for (auto iMesh = m_meshes.begin(); iMesh != m_meshes.end(); ++iMesh) {
        if (iMesh->get() == &mesh) {
            m_meshes.erase(iMesh);
            break;
        }
    }
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

vk::ImageView RenderScene::Impl::renderedImageView(uint32_t cameraIndex) const
{
    if (!m_initialized) {
        logger.warning("magma.vulkan.render-scenes.render-scene")
            << "Trying to access image view " << cameraIndex << " but not initialized." << std::endl;
        return nullptr;
    }

    if (cameraIndex >= m_cameraBundles.size()) {
        logger.warning("magma.vulkan.render-scenes.render-scene")
            << "Trying to access image view " << cameraIndex << " but only " << m_cameraBundles.size() << " added." << std::endl;
        return nullptr;
    }

    return m_cameraBundles[cameraIndex].deepDeferredStage->imageView();
}

//---- Internal interface

void RenderScene::Impl::updateCamera(uint32_t cameraId)
{
    updateStages(cameraId);

    // Present stage of RenderEngine is not right anymore, has the renderedImageView is no more
    m_engine.updateView(*m_cameraBundles[cameraId].camera);
}

void RenderScene::Impl::fallbackMaterial(std::unique_ptr<Material>&& material)
{
    m_fallbackMaterial = std::move(material);

    if (m_initialized) {
        m_fallbackMaterial->impl().init();
    }
}

//---- Internal

void RenderScene::Impl::initStages()
{
    logger.info("magma.vulkan.render-scenes.render-scene") << "Initializing render stages." << std::endl;
    logger.log().tab(1);

    for (auto cameraId = 0u; cameraId < m_cameraBundles.size(); ++cameraId) {
        auto& cameraBundle = m_cameraBundles[cameraId];
        cameraBundle.deepDeferredStage->init(cameraId);
        updateCamera(cameraId);
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
        mesh->interfaceImpl().init();
    }

    for (auto& light : m_lights) {
        light->interfaceImpl().init();
    }
}

void RenderScene::Impl::updateStages(uint32_t cameraId)
{
    auto& cameraBundle = m_cameraBundles[cameraId];
    auto& deepDeferredStage = *cameraBundle.deepDeferredStage;

    // Extent update
    const auto& extent = cameraBundle.camera->interfaceImpl().renderExtent();
    deepDeferredStage.update(extent);
}
