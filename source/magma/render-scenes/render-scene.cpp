#include <lava/magma/render-scenes/render-scene.hpp>

#include <lava/magma/material.hpp>

#include "../vulkan/render-scenes/render-scene-impl.hpp"

using namespace lava::magma;

RenderScene::RenderScene(RenderEngine& engine)
{
    m_impl = new Impl(engine, *this);

    // Fallback material
    auto fallbackMaterial = std::make_unique<Material>(*this, "fallback");
    m_impl->fallbackMaterial(std::move(fallbackMaterial));
}

RenderScene::~RenderScene()
{
    delete m_impl;
}

$pimpl_method(RenderScene, void, rendererType, RendererType, rendererType);

// Adders
void RenderScene::add(std::unique_ptr<ICamera>&& camera)
{
    m_impl->add(std::move(camera));
}

void RenderScene::add(std::unique_ptr<Material>&& material)
{
    m_impl->add(std::move(material));
}

void RenderScene::add(std::unique_ptr<Texture>&& texture)
{
    m_impl->add(std::move(texture));
}

$pimpl_method(RenderScene, void, add, Mesh&, mesh);

void RenderScene::add(std::unique_ptr<ILight>&& light)
{
    m_impl->add(std::move(light));
}

// Removers
$pimpl_method(RenderScene, void, remove, const Mesh&, mesh);
$pimpl_method(RenderScene, void, remove, const Material&, material);
$pimpl_method(RenderScene, void, remove, const Texture&, texture);

// Environment
$pimpl_method(RenderScene, void, environmentTexture, Texture*, texture);
