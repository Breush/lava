#include <lava/magma/render-engine.hpp>

#include <lava/chamber/macros.hpp>
#include <lava/magma/materials/rm-material.hpp>

#include "./vulkan/render-engine-impl.hpp"

using namespace lava::magma;

$pimpl_class(RenderEngine);

$pimpl_method(RenderEngine, void, draw);
$pimpl_method(RenderEngine, uint32_t, registerMaterial, const std::string&, shaderImplementation);

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
