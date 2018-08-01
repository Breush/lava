#include "./window-impl.hpp"

using namespace lava::crater;

Window::Impl::Impl(VideoMode mode, const std::string& title)
    : IWindowImpl(mode)
{
}

lava::WsHandle Window::Impl::handle() const
{
    return WsHandle();
}

void Window::Impl::processEvents() {}
