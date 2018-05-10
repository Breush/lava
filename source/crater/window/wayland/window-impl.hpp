#pragma once

#include "../../interfaces/window-impl.hpp"
#include <lava/crater/window.hpp>

#include <glm/vec2.hpp>

#error "Native wayland windows are currently implemented."

namespace lava::crater {
    /**
     * Wayland-based lava::Window.
     */
    class Window::Impl final : public IWindowImpl {
    public:
        Impl(VideoMode mode, const std::string& title);

        // IWindowImpl
        WsHandle handle() const override final;

    protected:
        // IWindowImpl
        virtual void processEvents() override final;
    };
}
