#include <lava/magma/render-engine.hpp>

#include <lava/magma/cameras/i-camera.hpp>

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
$pimpl_method(RenderEngine, uint32_t, registerMaterialFromFile, const std::string&, hrid, const fs::Path&, shaderPath);

uint32_t RenderEngine::addView(ICamera& camera, IRenderTarget& renderTarget, Viewport viewport)
{
    return addView(camera.renderImage(), renderTarget, viewport);
}

$pimpl_method(RenderEngine, uint32_t, addView, RenderImage, renderImage, IRenderTarget&, renderTarget, Viewport, viewport);
$pimpl_method(RenderEngine, void, removeView, uint32_t, viewId);

//----- Adders

void RenderEngine::add(std::unique_ptr<RenderScene>&& renderScene)
{
    m_impl->add(std::move(renderScene));
}

void RenderEngine::add(std::unique_ptr<IRenderTarget>&& renderTarget)
{
    m_impl->add(std::move(renderTarget));
}

//----- VR

$pimpl_method(RenderEngine, std::optional<VrEvent>, vrPollEvent);
$pimpl_method_const(RenderEngine, bool, vrEnabled);
$pimpl_method_const(RenderEngine, bool, vrDeviceValid, VrDeviceType, deviceType);
$pimpl_method_const(RenderEngine, const glm::mat4&, vrDeviceTransform, VrDeviceType, deviceType);
$pimpl_method(RenderEngine, Mesh&, vrDeviceMesh, VrDeviceType, deviceType, RenderScene&, scene);

//----- Extra

$pimpl_method(RenderEngine, void, logTrackingOnce);
