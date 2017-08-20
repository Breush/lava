#include <lava/magma/render-targets/render-window.hpp>

#include <lava/chamber/macros.hpp>

#include "./vulkan/render-targets/render-window-impl.hpp"

using namespace lava::magma;
using namespace lava::crater;

$pimpl_class(RenderWindow, RenderEngine&, engine, VideoMode, mode, const std::string&, title);

// IRenderTarget
IRenderTarget::Impl& RenderWindow::interfaceImpl()
{
    return *m_impl;
}

$pimpl_method(RenderWindow, bool, pollEvent, Event&, event);
$pimpl_method(RenderWindow, void, close);

$pimpl_attribute_v(RenderWindow, WindowHandle, windowHandle);
$pimpl_attribute_v(RenderWindow, bool, opened);
$pimpl_property(RenderWindow, VideoMode, videoMode);
