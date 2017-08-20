#include "./render-scene-impl.hpp"

#include <lava/chamber/logger.hpp>

#include "../render-engine-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

RenderScene::Impl::Impl(RenderEngine& engine)
    : m_engine(engine.impl())
{
}

//----- IRenderScene

void RenderScene::Impl::init()
{
    initStages();
    updateStages();
    initResources();
}

void RenderScene::Impl::extent(Extent2d extent)
{
    m_extent.width = extent.width;
    m_extent.height = extent.height;
    updateStages();
}

void RenderScene::Impl::render(vk::CommandBuffer commandBuffer)
{
    m_gBuffer.render(commandBuffer);
    m_epiphany.render(commandBuffer);
}

void RenderScene::Impl::add(std::unique_ptr<ICamera>&& camera)
{
    if (m_initialized) {
        camera->init();
    }

    m_cameras.emplace_back(std::move(camera));
}

void RenderScene::Impl::add(std::unique_ptr<IMaterial>&& material)
{
    if (m_initialized) {
        material->init();
    }

    m_materials.emplace_back(std::move(material));
}

void RenderScene::Impl::add(std::unique_ptr<IMesh>&& mesh)
{
    if (m_initialized) {
        mesh->init();
    }

    m_meshes.emplace_back(std::move(mesh));
}

void RenderScene::Impl::add(std::unique_ptr<IPointLight>&& pointLight)
{
    if (m_initialized) {
        pointLight->init();
    }

    m_pointLights.emplace_back(std::move(pointLight));
}

//---- Internal

void RenderScene::Impl::initStages()
{
    logger.info("magma.vulkan.render-scenes.render-scene") << "Initializing render stages." << std::endl;
    logger.log().tab(1);

    m_gBuffer.init();
    m_epiphany.init();

    m_initialized = true;

    logger.log().tab(-1);
}

void RenderScene::Impl::updateStages()
{
    if (!m_initialized) return;

    logger.info("magma.vulkan.render-scenes.render-scene") << "Updating render stages." << std::endl;
    logger.log().tab(1);

    m_gBuffer.update(m_extent);
    m_epiphany.update(m_extent);

    // Set-up
    m_epiphany.normalImageView(m_gBuffer.normalImageView(), m_engine.dummySampler());
    m_epiphany.albedoImageView(m_gBuffer.albedoImageView(), m_engine.dummySampler());
    m_epiphany.ormImageView(m_gBuffer.ormImageView(), m_engine.dummySampler());
    m_epiphany.depthImageView(m_gBuffer.depthImageView(), m_engine.dummySampler());

    logger.log().tab(-1);
}

void RenderScene::Impl::initResources()
{
    for (auto& camera : m_cameras) {
        camera->init();
    }

    for (auto& material : m_materials) {
        material->init();
    }

    for (auto& mesh : m_meshes) {
        mesh->init();
    }

    for (auto& pointLight : m_pointLights) {
        pointLight->init();
    }
}
