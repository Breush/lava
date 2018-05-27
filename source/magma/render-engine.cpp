#include <lava/magma/render-engine.hpp>

#include <fstream>
#include <lava/core/macros.hpp>
#include <sstream>

#include "./vulkan/render-engine-impl.hpp"

using namespace lava::magma;

RenderEngine::RenderEngine()
{
    m_impl = new RenderEngine::Impl();

    // Register fallback material
    // @todo Should be inlined as const string somehow
    std::ifstream fileStream("./data/shaders/materials/fallback-material.simpl");
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    registerMaterial("fallback", buffer.str(), {});
}

RenderEngine::~RenderEngine()
{
    delete m_impl;
}

$pimpl_method(RenderEngine, void, update);
$pimpl_method(RenderEngine, void, draw);
$pimpl_method(RenderEngine, uint32_t, registerMaterial, const std::string&, hrid, const std::string&, shaderImplementation,
              const UniformDefinitions&, uniformDefinitions);
$pimpl_method(RenderEngine, uint32_t, registerMaterialFromFile, const std::string&, hrid, const fs::Path&, shaderPath,
              const UniformDefinitions&, uniformDefinitions);

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
