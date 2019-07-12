#include <lava/magma/render-scenes/render-scene.hpp>

#include <lava/magma/material.hpp>

#include "../vulkan/render-scenes/render-scene-impl.hpp"

using namespace lava::magma;

RenderScene::RenderScene(RenderEngine& engine)
    : m_engine(engine)
{
    m_impl = new Impl(engine, *this);

    // Fallback material
    auto fallbackMaterial = m_materialAllocator.allocate<Material>(*this, "fallback");
    m_impl->add(*fallbackMaterial);
    m_impl->fallbackMaterial(*fallbackMaterial);
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

$pimpl_method(RenderScene, void, add, Material&, material);
$pimpl_method(RenderScene, void, add, Texture&, texture);
$pimpl_method(RenderScene, void, add, Mesh&, mesh);

void RenderScene::add(std::unique_ptr<ILight>&& light)
{
    m_impl->add(std::move(light));
}

// Removers
$pimpl_method(RenderScene, void, remove, const Material&, material);
$pimpl_method(RenderScene, void, remove, const Texture&, texture);
$pimpl_method(RenderScene, void, remove, const Mesh&, mesh);

// Environment
$pimpl_method(RenderScene, void, environmentTexture, Texture*, texture);
