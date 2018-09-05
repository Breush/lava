#include "./window-impl.hpp"

#include <functional>
#include <lava/chamber/logger.hpp>

using namespace lava::crater;
using namespace lava::chamber;

Window::Impl::Impl(VideoMode mode, const std::string& title)
    : IWindowImpl(mode)
{
    // Connect display
    m_display = wl_display_connect(nullptr);
    if (m_display == nullptr) {
        logger.error("crater.wayland.window") << "Unable to connect to display." << std::endl;
    }

    // Set up compositor
    static wl_registry_listener registryListener;
    registryListener.global = registryGlobal;
    registryListener.global_remove = registryGlobalRemove;

    m_registry = wl_display_get_registry(m_display);
    wl_registry_add_listener(m_registry, &registryListener, this);

    wl_display_dispatch(m_display);
    wl_display_roundtrip(m_display);

    if (!m_compositor || !m_shell || !m_seat) {
        logger.error("crater.wayland.window") << "Unable to find compositor, shell or seat." << std::endl;
    }

    // ----- Set-up window

    // Surface
    m_surface = wl_compositor_create_surface(m_compositor);
    if (m_surface == nullptr) {
        logger.error("crater.wayland.window") << "Unable to create surface." << std::endl;
    }

    // Shell surface
    m_shellSurface = wl_shell_get_shell_surface(m_shell, m_surface);
    if (m_shellSurface == nullptr) {
        logger.error("crater.wayland.window") << "Unable to create shell surface." << std::endl;
    }
    wl_shell_surface_set_toplevel(m_shellSurface);

    static wl_shell_surface_listener shellSurfaceListener;
    shellSurfaceListener.ping = shellSurfacePing;
    shellSurfaceListener.configure = shellSurfaceConfigure;
    shellSurfaceListener.popup_done = shellSurfacePopupDone;
    wl_shell_surface_add_listener(m_shellSurface, &shellSurfaceListener, nullptr);
    wl_shell_surface_set_toplevel(m_shellSurface);
    wl_shell_surface_set_title(m_shellSurface, title.c_str());

    // @fixme Looks there is no default window decoration
    // with wayland. Won't do ours here. And won't expose buffers nor EGL.
    // Just be sure it works with magma/vulkan.
}

Window::Impl::~Impl()
{
    if (m_pointer) wl_pointer_destroy(m_pointer);
    if (m_keyboard) wl_keyboard_destroy(m_keyboard);

    wl_shell_surface_destroy(m_shellSurface);
    wl_surface_destroy(m_surface);
    wl_seat_destroy(m_seat);
    wl_shell_destroy(m_shell);
    wl_compositor_destroy(m_compositor);
    wl_registry_destroy(m_registry);
    wl_display_disconnect(m_display);
}

lava::WsHandle Window::Impl::handle() const
{
    WsHandle handle;
    handle.wayland.display = m_display;
    handle.wayland.surface = m_surface;
    return handle;
}

// ----- IWindowImpl

void Window::Impl::processEvents()
{
    while (wl_display_prepare_read(m_display) != 0) {
        wl_display_dispatch_pending(m_display);
    }
    wl_display_flush(m_display);
    wl_display_read_events(m_display);
    wl_display_dispatch_pending(m_display);
}

// ----- Wayland

void Window::Impl::registryGlobal(uint32_t id, std::string interface)
{
    if (interface == "wl_compositor") {
        m_compositor = reinterpret_cast<wl_compositor*>(wl_registry_bind(m_registry, id, &wl_compositor_interface, 1));
    }
    else if (interface == "wl_shell") {
        m_shell = reinterpret_cast<wl_shell*>(wl_registry_bind(m_registry, id, &wl_shell_interface, 1));
    }
    else if (interface == "wl_seat") {
        m_seat = reinterpret_cast<wl_seat*>(wl_registry_bind(m_registry, id, &wl_seat_interface, 1));

        static wl_seat_listener seatListener;
        seatListener.capabilities = seatCapabilities;
        wl_seat_add_listener(m_seat, &seatListener, this);
    }
}

void Window::Impl::seatCapabilities(uint32_t capabilities)
{
    if ((capabilities & WL_SEAT_CAPABILITY_POINTER) && !m_pointer) {
        m_pointer = wl_seat_get_pointer(m_seat);
        // @todo Do some wl_pointer_add_listener
    }
    else if (!(capabilities & WL_SEAT_CAPABILITY_POINTER) && m_pointer) {
        wl_pointer_destroy(m_pointer);
        m_pointer = nullptr;
    }

    if ((capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && !m_keyboard) {
        m_keyboard = wl_seat_get_keyboard(m_seat);
        static wl_keyboard_listener keyboardListener;
        keyboardListener.keymap = keyboardKeymap;
        keyboardListener.enter = keyboardEnter;
        keyboardListener.leave = keyboardLeave;
        keyboardListener.key = keyboardKey;
        keyboardListener.modifiers = keyboardModifiers;
        wl_keyboard_add_listener(m_keyboard, &keyboardListener, this);
    }
    else if (!(capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && m_keyboard) {
        wl_keyboard_destroy(m_keyboard);
        m_keyboard = nullptr;
    }
}

void Window::Impl::keyboardKey(uint32_t /* key */, uint32_t /* state */)
{
    // @todo Push the event into m_events.
}

// ----- Wayland statics

void Window::Impl::registryGlobal(void* data, wl_registry*, uint32_t id, const char* interface, uint32_t)
{
    auto windowImpl = reinterpret_cast<Window::Impl*>(data);
    windowImpl->registryGlobal(id, interface);
}

void Window::Impl::shellSurfacePing(void*, wl_shell_surface* shellSurface, uint32_t serial)
{
    wl_shell_surface_pong(shellSurface, serial);
}

void Window::Impl::seatCapabilities(void* data, wl_seat*, uint32_t capabilities)
{
    auto windowImpl = reinterpret_cast<Window::Impl*>(data);
    windowImpl->seatCapabilities(capabilities);
}

void Window::Impl::keyboardKey(void* data, struct wl_keyboard*, uint32_t, uint32_t, uint32_t key, uint32_t state)
{
    auto windowImpl = reinterpret_cast<Window::Impl*>(data);
    windowImpl->keyboardKey(key, state);
}
