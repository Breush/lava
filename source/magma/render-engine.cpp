#include <lava/magma/render-engine.hpp>

#include <lava/chamber/macros.hpp>

#include "./materials/fallback-material.hpp"
#include "./vulkan/render-engine-impl.hpp"

using namespace lava::magma;

RenderEngine::RenderEngine()
{
    m_impl = new RenderEngine::Impl();

    registerMaterial<magma::FallbackMaterial>();
}

RenderEngine::~RenderEngine()
{
}

$pimpl_method(RenderEngine, void, draw);
$pimpl_method(RenderEngine, uint32_t, registerMaterial, const std::string&, hrid, const std::string&, shaderImplementation);

$pimpl_method(RenderEngine, uint32_t, addView, ICamera&, camera, IRenderTarget&, renderTarget, Viewport, viewport);

//----- Adders

void RenderEngine::add(std::unique_ptr<IRenderScene>&& renderScene)
{
    m_impl->add(std::move(renderScene));
}

void RenderEngine::add(std::unique_ptr<IRenderTarget>&& renderTarget)
{
    m_impl->add(std::move(renderTarget));
}
