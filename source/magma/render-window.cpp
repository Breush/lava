#include <lava/magma/render-window.hpp>

#include <lava/chamber/macros.hpp>

#include "./vulkan/render-window-impl.hpp"

using namespace lava::magma;
using namespace lava::crater;

$pimpl_class(RenderWindow, RenderEngine&, engine, VideoMode, mode, const std::string&, title);

$pimpl_method(RenderWindow, void, init, UserData, data);
$pimpl_method(RenderWindow, void, prepare);
$pimpl_method_const(RenderWindow, void, draw, UserData, data);
$pimpl_method(RenderWindow, IRenderTarget::UserData, data);

$pimpl_method(RenderWindow, bool, pollEvent, Event&, event);
$pimpl_method(RenderWindow, void, close);

$pimpl_attribute_v(RenderWindow, WindowHandle, windowHandle);
$pimpl_attribute_v(RenderWindow, bool, opened);
$pimpl_property(RenderWindow, VideoMode, videoMode);
