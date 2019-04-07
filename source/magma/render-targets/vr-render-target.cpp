#include <lava/magma/render-targets/vr-render-target.hpp>

#include "../vulkan/render-targets/vr-render-target-impl.hpp"

using namespace lava::magma;

$pimpl_class(VrRenderTarget, RenderEngine&, engine);

$pimpl_method(VrRenderTarget, void, bindScene, RenderScene&, scene);

// IRenderTarget
IRenderTarget::Impl& VrRenderTarget::interfaceImpl()
{
    return *m_impl;
}
