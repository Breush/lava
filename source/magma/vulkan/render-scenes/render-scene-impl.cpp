#include "./render-scene-impl.hpp"

#include <lava/chamber/logger.hpp>

#include "../cameras/i-camera-impl.hpp"
#include "../lights/i-light-impl.hpp"
#include "../materials/i-material-impl.hpp"
#include "../meshes/i-mesh-impl.hpp"
#include "../render-engine-impl.hpp"
#include "../stages/epiphany.hpp"
#include "../stages/g-buffer.hpp"

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

    // GBuffer common descriptors
    m_cameraDescriptorHolder.init({1}, {}, 16, vk::ShaderStageFlagBits::eVertex);
    m_materialDescriptorHolder.init({1}, {1, 1, 1}, 128, vk::ShaderStageFlagBits::eFragment);
    m_meshDescriptorHolder.init({1}, {}, 128, vk::ShaderStageFlagBits::eVertex);

    initStages();
    initResources();
}

void RenderScene::Impl::render(vk::CommandBuffer commandBuffer)
{
    for (const auto& cameraBundle : m_cameraBundles) {
        cameraBundle.gBuffer->render(commandBuffer);
        cameraBundle.epiphany->render(commandBuffer);
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
    cameraBundle.gBuffer = std::make_unique<GBuffer>(*this);
    cameraBundle.epiphany = std::make_unique<Epiphany>(*this);

    if (m_initialized) {
        cameraBundle.camera->interfaceImpl().init(cameraId);
        cameraBundle.gBuffer->init(cameraId);
        cameraBundle.epiphany->init(cameraId);
        updateStages(cameraId);
    }

    logger.log().tab(-1);
}

void RenderScene::Impl::add(std::unique_ptr<IMaterial>&& material)
{
    if (m_initialized) {
        material->interfaceImpl().init();
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

    return m_cameraBundles[cameraIndex].epiphany->imageView();
}

//---- Internal interface

void RenderScene::Impl::updateCamera(uint32_t cameraId)
{
    updateStages(cameraId);

    // Present stage of RenderEngine is not right anymore, has the renderedImageView is no more
    m_engine.updateView(*m_cameraBundles[cameraId].camera);
}

//---- Internal

void RenderScene::Impl::initStages()
{
    logger.info("magma.vulkan.render-scenes.render-scene") << "Initializing render stages." << std::endl;
    logger.log().tab(1);

    for (auto cameraId = 0u; cameraId < m_cameraBundles.size(); ++cameraId) {
        auto& cameraBundle = m_cameraBundles[cameraId];
        cameraBundle.gBuffer->init(cameraId);
        cameraBundle.epiphany->init(cameraId);
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

    for (auto& material : m_materials) {
        material->interfaceImpl().init();
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
    auto& gBuffer = *cameraBundle.gBuffer;
    auto& epiphany = *cameraBundle.epiphany;

    // Extent update
    const auto& extent = cameraBundle.camera->interfaceImpl().renderExtent();
    gBuffer.update(extent);
    epiphany.update(extent);

    // Image views set-up
    epiphany.normalImageView(gBuffer.normalImageView(), m_engine.dummySampler());
    epiphany.albedoImageView(gBuffer.albedoImageView(), m_engine.dummySampler());
    epiphany.ormImageView(gBuffer.ormImageView(), m_engine.dummySampler());
    epiphany.depthImageView(gBuffer.depthImageView(), m_engine.dummySampler());
}
