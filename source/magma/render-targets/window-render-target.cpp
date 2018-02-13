#include <lava/magma/render-targets/window-render-target.hpp>

#include <lava/chamber/macros.hpp>

#include "../vulkan/render-targets/window-render-target-impl.hpp"

using namespace lava::magma;
using namespace lava::crater;

$pimpl_class(WindowRenderTarget, RenderEngine&, engine, WsHandle, handle, const Extent2d&, extent);

// IRenderTarget
IRenderTarget::Impl& WindowRenderTarget::interfaceImpl()
{
    return *m_impl;
}

$pimpl_method_const(WindowRenderTarget, lava::Extent2d, extent);
$pimpl_method(WindowRenderTarget, void, extent, const Extent2d&, extent);
$pimpl_attribute_v(WindowRenderTarget, lava::WsHandle, handle);
