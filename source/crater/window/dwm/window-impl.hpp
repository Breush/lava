#pragma once

#include "../../interfaces/window-impl.hpp"
#include <lava/crater/window.hpp>

#include <glm/vec2.hpp>
#include <windows.h>

namespace lava::crater {
    /**
     * Windows' DWM-based lava::Window.
     */
    class Window::Impl final : public IWindowImpl {
    public:
        Impl(VideoMode mode, const std::string& title);

        // IWindowImpl
        WindowHandle windowHandle() const override final;

        // Exploited by onWindowMessage.
        bool processEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

    protected:
        // IWindowImpl
        virtual void processEvents() override final;

    private:
        HWND m_hwnd = nullptr;
        HINSTANCE m_hinstance = nullptr;
    };
}
