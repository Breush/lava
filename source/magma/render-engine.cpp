#include <lava/magma/render-engine.hpp>

#include <lava/magma/camera.hpp>

#include "./aft-vulkan/scene-aft.hpp"
#include "./vulkan/render-engine-impl.hpp"

using namespace lava;
using namespace lava::magma;

RenderEngine::RenderEngine()
{
    m_impl = new RenderEngine::Impl(*this);

    // Register fallback material
    // @todo Should be inlined as const string somehow
    registerMaterialFromFile("fallback", "./data/shaders/materials/fallback-material.shmag");
}

RenderEngine::~RenderEngine()
{
    delete m_impl;
}

$pimpl_method(RenderEngine, void, update);
$pimpl_method(RenderEngine, void, draw);
$pimpl_method_const(RenderEngine, const MaterialInfo&, materialInfo, const std::string&, hrid);
$pimpl_method_const(RenderEngine, const MaterialInfo*, materialInfoIfExists, const std::string&, hrid);
$pimpl_method(RenderEngine, uint32_t, registerMaterialFromFile, const std::string&, hrid, const fs::Path&, shaderPath);

uint32_t RenderEngine::addView(Camera& camera, IRenderTarget& renderTarget, Viewport viewport)
{
    return addView(camera.renderImage(), renderTarget, viewport);
}

$pimpl_method(RenderEngine, uint32_t, addView, RenderImage, renderImage, IRenderTarget&, renderTarget, Viewport, viewport);
$pimpl_method(RenderEngine, void, removeView, uint32_t, viewId);

//----- Makers

// :RuntimeAft

Scene& RenderEngine::makeScene()
{
    constexpr const auto size = sizeof(std::aligned_union<0, Scene>::type) + sizeof(SceneAft);
    auto resource = m_sceneAllocator.allocateSized<Scene>(size, *this);
    add(*resource);
    return *resource;
}

//----- Adders

$pimpl_method(RenderEngine, void, add, Scene&, scene);

void RenderEngine::add(std::unique_ptr<IRenderTarget>&& renderTarget)
{
    m_impl->add(std::move(renderTarget));
}

//----- Extra

$pimpl_method(RenderEngine, void, logTrackingOnce);
