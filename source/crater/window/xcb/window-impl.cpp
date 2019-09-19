#include "./window-impl.hpp"

#include <cstring>
#include <fcntl.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <xcb/xcb_keysyms.h>

// @note Most on this code has been written thanks to
// https://www.x.org/releases/X11R7.6/doc/libxcb/tutorial/index.html

// @note These horizontal mouse button indices
// are not always defined...
#if !defined(XCB_BUTTON_INDEX_6)
#define XCB_BUTTON_INDEX_6 6
#endif
#if !defined(XCB_BUTTON_INDEX_7)
#define XCB_BUTTON_INDEX_7 7
#endif

using namespace lava;

namespace {
    xcb_key_symbols_t* g_keySymbols = nullptr;

    inline xcb_intern_atom_reply_t* internAtomHelper(xcb_connection_t* conn, bool only_if_exists, const char* str)
    {
        auto cookie = xcb_intern_atom(conn, only_if_exists, strlen(str), str);
        return xcb_intern_atom_reply(conn, cookie, nullptr);
    }

    // Got list from https://github.com/substack/node-keysym/blob/master/data/keysyms.txt
    Key keyEventToKey(xcb_key_press_event_t& keyEvent)
    {
        xcb_keysym_t keySym = xcb_key_press_lookup_keysym(g_keySymbols, &keyEvent, 0);

        switch (keySym) {
        case 'a': return Key::A;
        case 'b': return Key::B;
        case 'c': return Key::C;
        case 'd': return Key::D;
        case 'e': return Key::E;
        case 'f': return Key::F;
        case 'g': return Key::G;
        case 'h': return Key::H;
        case 'i': return Key::I;
        case 'j': return Key::J;
        case 'k': return Key::K;
        case 'l': return Key::L;
        case 'm': return Key::M;
        case 'n': return Key::N;
        case 'o': return Key::O;
        case 'p': return Key::P;
        case 'q': return Key::Q;
        case 'r': return Key::R;
        case 's': return Key::S;
        case 't': return Key::T;
        case 'u': return Key::U;
        case 'v': return Key::V;
        case 'w': return Key::W;
        case 'x': return Key::X;
        case 'y': return Key::Y;
        case 'z': return Key::Z;
        case 0xff1b: return Key::Escape;
        case 0xffbe: return Key::F1;
        case 0xffbf: return Key::F2;
        case 0xffc0: return Key::F3;
        case 0xffc1: return Key::F4;
        case 0xffc2: return Key::F5;
        case 0xffc3: return Key::F6;
        case 0xffc4: return Key::F7;
        case 0xffc5: return Key::F8;
        case 0xffc6: return Key::F9;
        case 0xffc7: return Key::F10;
        case 0xffc8: return Key::F11;
        case 0xffc9: return Key::F12;
        case 0xff51: return Key::Left;
        case 0xff52: return Key::Up;
        case 0xff53: return Key::Right;
        case 0xff54: return Key::Down;
        case 0xffe1: return Key::LeftShift;
        case 0xffe2: return Key::RightShift;
        case 0xffe3: return Key::LeftControl;
        case 0xffe4: return Key::RightControl;
        case 0xffe9: return Key::LeftAlt;
        case 0xffea: return Key::RightAlt;
        }

        return Key::Unknown;
    }
}

using namespace crater;
using namespace chamber;

Window::Impl::Impl(VideoMode mode, const std::string& title)
    : IWindowImpl(mode)
{
    initXcbConnection();
    setupWindow(mode, title);

    // @todo If not allocated?
    // Probably more secure to put that as a class member for now.
    g_keySymbols = xcb_key_symbols_alloc(m_connection);
}

Window::Impl::~Impl()
{
    free(m_hintsReply);
    free(m_protocolsReply);
    free(m_deleteWindowReply);
}

void Window::Impl::fullscreen(bool fullscreen)
{
    if (m_fullscreen == fullscreen) return;
    m_fullscreen = fullscreen;

    // @note When fullscreen was on, we need to unmap
    // the window first, otherwise it will be kept fullscreen.
    if (!m_fullscreen) {
        xcb_unmap_window(m_connection, m_window);
    }
    // @note When going fullscreen mode, we first store the dimensions of the window,
    // so that we can go back seemlessly for the user.
    else {
        auto geometryCookie = xcb_get_geometry(m_connection, m_window);
        auto geometryReply = xcb_get_geometry_reply(m_connection, geometryCookie, nullptr);
        m_extentBeforeFullscreen.width = geometryReply->width;
        m_extentBeforeFullscreen.height = geometryReply->height;
        free(geometryReply);
    }

    // Configuration
    uint32_t configuration[] = {
        0u,                                                                          // WINDOW_X
        0u,                                                                          // WINDOW_Y
        m_fullscreen ? m_screen->width_in_pixels : m_extentBeforeFullscreen.width,   // WINDOW_WIDTH
        m_fullscreen ? m_screen->height_in_pixels : m_extentBeforeFullscreen.height, // WINDOW_HEIGHT
        XCB_STACK_MODE_ABOVE,                                                        // STACK_MODE
    };

    auto configurationFlags = XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT       // dimensions
                              | XCB_CONFIG_WINDOW_STACK_MODE;                          // order
    if (m_fullscreen) configurationFlags |= XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y; // position
    xcb_configure_window(m_connection, m_window, configurationFlags, configuration + (m_fullscreen ? 0u : 2u));

    if (!m_fullscreen) {
        xcb_map_window(m_connection, m_window);
    }
}

WsHandle Window::Impl::handle() const
{
    return {m_connection, m_window};
}

// ----- Internal

void Window::Impl::initXcbConnection()
{
    const xcb_setup_t* setup;
    xcb_screen_iterator_t iter;

    int scr;
    m_connection = xcb_connect(nullptr, &scr);
    if (m_connection == nullptr) {
        logger.error("crater.window") << "Could not find a XCB connection.\n" << std::endl;
        exit(1);
    }

    setup = xcb_get_setup(m_connection);
    iter = xcb_setup_roots_iterator(setup);
    while (scr-- > 0) xcb_screen_next(&iter);
    m_screen = iter.data;
}

void Window::Impl::setupWindow(VideoMode mode, const std::string& title)
{
    uint32_t value_mask, value_list[32];

    m_window = xcb_generate_id(m_connection);

    value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    value_list[0] = m_screen->black_pixel;
    value_list[1] = XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_EXPOSURE
                    | XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_BUTTON_PRESS
                    | XCB_EVENT_MASK_BUTTON_RELEASE;
    value_list[2] = 0;

    xcb_create_window(m_connection, XCB_COPY_FROM_PARENT, m_window, m_screen->root, 0, 0, mode.width, mode.height, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, m_screen->root_visual, value_mask, value_list);

    // Stored replies
    m_hintsReply = internAtomHelper(m_connection, true, "_MOTIF_WM_HINTS");
    m_protocolsReply = internAtomHelper(m_connection, true, "WM_PROTOCOLS");
    m_deleteWindowReply = internAtomHelper(m_connection, false, "WM_DELETE_WINDOW");

    // Enable window destroyed notifications
    xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, m_window, m_protocolsReply->atom, XCB_ATOM_ATOM, 32, 1,
                        &m_deleteWindowReply->atom);

    // Set title
    xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, m_window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, title.size(),
                        title.c_str());

    xcb_map_window(m_connection, m_window);
    xcb_flush(m_connection);
}

void Window::Impl::processEvents()
{
    xcb_generic_event_t* event;
    while ((event = xcb_poll_for_event(m_connection))) {
        processEvent(*event);
        free(event);
    }
}

bool Window::Impl::processEvent(xcb_generic_event_t& windowEvent)
{
    xcb_flush(m_connection);

    switch (windowEvent.response_type & 0x7f) {
    case XCB_DESTROY_NOTIFY: {
        break;
    }

    case XCB_CLIENT_MESSAGE: {
        auto messageEvent = reinterpret_cast<xcb_client_message_event_t&>(windowEvent);
        if (messageEvent.data.data32[0] != m_deleteWindowReply->atom) break;

        WsEvent event;
        event.type = WsEventType::WindowClosed;
        pushEvent(event);
        break;
    }

    case XCB_CONFIGURE_NOTIFY: {
        auto configureEvent = reinterpret_cast<xcb_configure_notify_event_t&>(windowEvent);
        if (m_extent.width == configureEvent.width && m_extent.height == configureEvent.height) break;

        WsEvent event;
        event.type = WsEventType::WindowResized;
        event.windowSize.width = configureEvent.width;
        event.windowSize.height = configureEvent.height;
        pushEvent(event);

        m_extent.width = configureEvent.width;
        m_extent.height = configureEvent.height;
        break;
    }

    case XCB_BUTTON_PRESS: {
        auto buttonEvent = reinterpret_cast<xcb_button_press_event_t&>(windowEvent);

        WsEvent event;

        // Classic buttons
        if (buttonEvent.detail <= XCB_BUTTON_INDEX_3) {
            event.type = WsEventType::MouseButtonPressed;
            event.mouseButton.x = buttonEvent.event_x;
            event.mouseButton.y = buttonEvent.event_y;
            if (buttonEvent.detail == XCB_BUTTON_INDEX_1)
                event.mouseButton.which = MouseButton::Left;
            else if (buttonEvent.detail == XCB_BUTTON_INDEX_2)
                event.mouseButton.which = MouseButton::Middle;
            else if (buttonEvent.detail == XCB_BUTTON_INDEX_3)
                event.mouseButton.which = MouseButton::Right;
        }
        // Vertical mouse wheel buttons
        else if (buttonEvent.detail <= XCB_BUTTON_INDEX_5) {
            event.type = WsEventType::MouseWheelScrolled;
            event.mouseWheel.which = MouseWheel::Vertical;
            event.mouseWheel.x = buttonEvent.event_x;
            event.mouseWheel.y = buttonEvent.event_y;
            event.mouseWheel.delta = (buttonEvent.detail == XCB_BUTTON_INDEX_4) ? 1.f : -1.f;
        }
        // Horizontal mouse wheel buttons
        else if (buttonEvent.detail <= XCB_BUTTON_INDEX_7) {
            event.type = WsEventType::MouseWheelScrolled;
            event.mouseWheel.which = MouseWheel::Horizontal;
            event.mouseWheel.x = buttonEvent.event_x;
            event.mouseWheel.y = buttonEvent.event_y;
            event.mouseWheel.delta = (buttonEvent.detail == XCB_BUTTON_INDEX_4) ? 1.f : -1.f;
        }
        else {
            logger.warning("crater.xcb.window")
                << "Unknown buttonEvent.detail during BUTTON_PRESS: " << static_cast<int>(buttonEvent.detail) << std::endl;
            break;
        }
        pushEvent(event);
        break;
    }

    case XCB_BUTTON_RELEASE: {
        auto buttonEvent = reinterpret_cast<xcb_button_release_event_t&>(windowEvent);

        WsEvent event;
        event.type = WsEventType::MouseButtonReleased;
        event.mouseButton.x = buttonEvent.event_x;
        event.mouseButton.y = buttonEvent.event_y;

        // Classic buttons
        if (buttonEvent.detail == XCB_BUTTON_INDEX_1)
            event.mouseButton.which = MouseButton::Left;
        else if (buttonEvent.detail == XCB_BUTTON_INDEX_2)
            event.mouseButton.which = MouseButton::Middle;
        else if (buttonEvent.detail == XCB_BUTTON_INDEX_3)
            event.mouseButton.which = MouseButton::Right;
        // Mouse wheel buttons
        else if (buttonEvent.detail <= XCB_BUTTON_INDEX_7) {
            // Nothing to do, scrolling is handled in BUTTON_PRESS
            break;
        }
        else {
            logger.warning("crater.xcb.window")
                << "Unknown buttonEvent.detail during BUTTON_RELEASE: " << static_cast<int>(buttonEvent.detail) << std::endl;
            break;
        }
        pushEvent(event);
        break;
    }

    case XCB_MOTION_NOTIFY: {
        auto motionEvent = reinterpret_cast<xcb_motion_notify_event_t&>(windowEvent);

        WsEvent event;
        event.type = WsEventType::MouseMoved;
        event.mouseMove.x = motionEvent.event_x;
        event.mouseMove.y = motionEvent.event_y;
        pushEvent(event);
        break;
    }

    case XCB_KEY_PRESS: {
        auto keyEvent = reinterpret_cast<xcb_key_press_event_t&>(windowEvent);

        WsEvent event;
        event.type = WsEventType::KeyPressed;
        event.key.which = keyEventToKey(keyEvent);
        pushEvent(event);
    } break;

    case XCB_KEY_RELEASE: {
        auto keyEvent = reinterpret_cast<xcb_key_release_event_t&>(windowEvent);

        WsEvent event;
        event.type = WsEventType::KeyReleased;
        event.key.which = keyEventToKey(keyEvent);
        pushEvent(event);
    } break;

    default: break;
    }

    return true;
}
