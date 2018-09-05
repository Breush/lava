#pragma once

#include "../../interfaces/window-impl.hpp"
#include <lava/crater/window.hpp>

#include <glm/vec2.hpp>
#include <wayland-client.h>

namespace lava::crater {
    /**
     * Wayland-based lava::Window.
     */
    class Window::Impl final : public IWindowImpl {
    public:
        Impl(VideoMode mode, const std::string& title);
        ~Impl();

        // IWindowImpl
        WsHandle handle() const override final;
        bool fullscreen() const override final { return false; }
        void fullscreen(bool) override final
        {
            // @fixme Implement Wayland fullscreen support.
        }

    protected:
        // IWindowImpl
        virtual void processEvents() override final;

        // Wayland
        void registryGlobal(uint32_t id, std::string interface);
        void seatCapabilities(uint32_t capabilities);
        void keyboardKey(uint32_t key, uint32_t state);

        // Wayland statics
        static void registryGlobal(void* data, wl_registry* registry, uint32_t id, const char* interface, uint32_t serial);
        static void registryGlobalRemove(void*, wl_registry*, uint32_t) {}
        static void shellSurfacePing(void* data, wl_shell_surface* shell_surface, uint32_t serial);
        static void shellSurfaceConfigure(void*, wl_shell_surface*, uint32_t, int32_t, int32_t) {}
        static void shellSurfacePopupDone(void*, wl_shell_surface*) {}
        static void seatCapabilities(void* data, wl_seat* seat, uint32_t capabilities);
        static void keyboardKeymap(void*, struct wl_keyboard*, uint32_t, int, uint32_t) {}
        static void keyboardEnter(void*, struct wl_keyboard*, uint32_t, struct wl_surface*, struct wl_array*) {}
        static void keyboardLeave(void*, struct wl_keyboard*, uint32_t, struct wl_surface*) {}
        static void keyboardKey(void* data, struct wl_keyboard*, uint32_t, uint32_t, uint32_t key, uint32_t state);
        static void keyboardModifiers(void*, struct wl_keyboard*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {}

    private:
        wl_display* m_display = nullptr;
        wl_compositor* m_compositor = nullptr;
        wl_shell* m_shell = nullptr;
        wl_registry* m_registry = nullptr;
        wl_surface* m_surface = nullptr;
        wl_shell_surface* m_shellSurface = nullptr;
        wl_seat* m_seat = nullptr;
        wl_pointer* m_pointer = nullptr;
        wl_keyboard* m_keyboard = nullptr;
    };
}
