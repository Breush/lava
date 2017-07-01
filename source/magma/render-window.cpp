#include <lava/magma/render-window.hpp>

#include <lava/chamber/macros.hpp>

#include "./vulkan/render-window-impl.hpp"

using namespace lava;

$pimpl_class(RenderWindow, crater::VideoMode, mode, const std::string&, title);

$pimpl_method(RenderWindow, void, init, RenderEngine&, engine);
$pimpl_method_const(RenderWindow, void, draw);
$pimpl_method(RenderWindow, void, refresh);

$pimpl_method(RenderWindow, bool, pollEvent, crater::Event&, event);
$pimpl_method(RenderWindow, void, close);

$pimpl_method_const(RenderWindow, crater::WindowHandle, windowHandle);
$pimpl_method_const(RenderWindow, bool, opened);

// @todo Have pimpl property
$pimpl_method_const(RenderWindow, crater::VideoMode, videoMode);
$pimpl_method(RenderWindow, void, videoMode, const crater::VideoMode&, mode);
