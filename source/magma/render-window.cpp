#include <lava/magma/render-window.hpp>

#include <lava/chamber/macros.hpp>

#include "./vulkan/render-window-impl.hpp"

using namespace lava::magma;
using namespace lava::crater;

$pimpl_class(RenderWindow, VideoMode, mode, const std::string&, title);

$pimpl_method(RenderWindow, void, init, RenderEngine&, engine);
$pimpl_method_const(RenderWindow, void, draw);
$pimpl_method(RenderWindow, void, refresh);

$pimpl_method(RenderWindow, bool, pollEvent, Event&, event);
$pimpl_method(RenderWindow, void, close);

$pimpl_method_const(RenderWindow, WindowHandle, windowHandle);
$pimpl_method_const(RenderWindow, bool, opened);

// @todo Have pimpl property
$pimpl_method_const(RenderWindow, VideoMode, videoMode);
$pimpl_method(RenderWindow, void, videoMode, const VideoMode&, mode);
