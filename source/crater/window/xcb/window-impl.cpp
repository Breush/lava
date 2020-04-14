#include "./window-impl.hpp"

#include <cstring>
#include <fcntl.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon-x11.h>

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
    constexpr uint32_t EVENT_MASK_FLAGS = XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_KEY_PRESS |
                                          XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                                          XCB_EVENT_MASK_POINTER_MOTION |
                                          XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE;

    inline xcb_intern_atom_reply_t* internAtomHelper(xcb_connection_t* conn, bool only_if_exists, const char* str)
    {
        auto cookie = xcb_intern_atom(conn, only_if_exists, strlen(str), str);
        return xcb_intern_atom_reply(conn, cookie, nullptr);
    }

    Key keysymToKey(xkb_keysym_t keysym)
    {
        switch (keysym) {
        case XKB_KEY_a: case XKB_KEY_A: return Key::A;
        case XKB_KEY_b: case XKB_KEY_B: return Key::B;
        case XKB_KEY_c: case XKB_KEY_C: return Key::C;
        case XKB_KEY_d: case XKB_KEY_D: return Key::D;
        case XKB_KEY_e: case XKB_KEY_E: return Key::E;
        case XKB_KEY_f: case XKB_KEY_F: return Key::F;
        case XKB_KEY_g: case XKB_KEY_G: return Key::G;
        case XKB_KEY_h: case XKB_KEY_H: return Key::H;
        case XKB_KEY_i: case XKB_KEY_I: return Key::I;
        case XKB_KEY_j: case XKB_KEY_J: return Key::J;
        case XKB_KEY_k: case XKB_KEY_K: return Key::K;
        case XKB_KEY_l: case XKB_KEY_L: return Key::L;
        case XKB_KEY_m: case XKB_KEY_M: return Key::M;
        case XKB_KEY_n: case XKB_KEY_N: return Key::N;
        case XKB_KEY_o: case XKB_KEY_O: return Key::O;
        case XKB_KEY_p: case XKB_KEY_P: return Key::P;
        case XKB_KEY_q: case XKB_KEY_Q: return Key::Q;
        case XKB_KEY_r: case XKB_KEY_R: return Key::R;
        case XKB_KEY_s: case XKB_KEY_S: return Key::S;
        case XKB_KEY_t: case XKB_KEY_T: return Key::T;
        case XKB_KEY_u: case XKB_KEY_U: return Key::U;
        case XKB_KEY_v: case XKB_KEY_V: return Key::V;
        case XKB_KEY_w: case XKB_KEY_W: return Key::W;
        case XKB_KEY_x: case XKB_KEY_X: return Key::X;
        case XKB_KEY_y: case XKB_KEY_Y: return Key::Y;
        case XKB_KEY_z: case XKB_KEY_Z: return Key::Z;
        case XKB_KEY_BackSpace: return Key::Backspace;
        case XKB_KEY_Escape: return Key::Escape;
        case XKB_KEY_F1: return Key::F1;
        case XKB_KEY_F2: return Key::F2;
        case XKB_KEY_F3: return Key::F3;
        case XKB_KEY_F4: return Key::F4;
        case XKB_KEY_F5: return Key::F5;
        case XKB_KEY_F6: return Key::F6;
        case XKB_KEY_F7: return Key::F7;
        case XKB_KEY_F8: return Key::F8;
        case XKB_KEY_F9: return Key::F9;
        case XKB_KEY_F10: return Key::F10;
        case XKB_KEY_F11: return Key::F11;
        case XKB_KEY_F12: return Key::F12;
        case XKB_KEY_Left: return Key::Left;
        case XKB_KEY_Up: return Key::Up;
        case XKB_KEY_Right: return Key::Right;
        case XKB_KEY_Down: return Key::Down;
        case XKB_KEY_Shift_L: return Key::LeftShift;
        case XKB_KEY_Shift_R: return Key::RightShift;
        case XKB_KEY_Control_L: return Key::LeftControl;
        case XKB_KEY_Control_R: return Key::RightControl;
        case XKB_KEY_Alt_L: return Key::LeftAlt;
        case XKB_KEY_Alt_R: return Key::RightAlt;
        case XKB_KEY_Delete: return Key::Delete;
        default: {
            // @note Debug to understand unhandled keys
            // char keysym_name[64];
            // xkb_keysym_get_name(keysym, keysym_name, sizeof(keysym_name));
            // std::cout << keysym_name << ": 0x" << std::hex << keysym << std::endl;
        }
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
    setupXkb();
}

Window::Impl::~Impl()
{
    xkb_state_unref(m_xkbState);
    xkb_keymap_unref(m_xkbKeymap);
    xkb_context_unref(m_xkbContext);

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

void Window::Impl::mouseHidden(bool mouseHidden)
{
    if (m_mouseHidden == mouseHidden) return;
    m_mouseHidden = mouseHidden;

    if (m_emptyCursor == XCB_NONE) {
        m_emptyCursor = xcb_generate_id(m_connection);
        xcb_pixmap_t pixmap = xcb_generate_id(m_connection);
        xcb_create_pixmap(m_connection, 1, pixmap, m_window, 1, 1);
        xcb_create_cursor(m_connection, m_emptyCursor, pixmap, pixmap,
                          0, 0, 0, 0, 0, 0, 0, 0);
        xcb_free_pixmap(m_connection, pixmap);
    }

    const uint32_t values = (mouseHidden) ? m_emptyCursor : XCB_NONE;
    xcb_change_window_attributes(m_connection, m_window, XCB_CW_CURSOR, &values);
    xcb_flush(m_connection);
}

void Window::Impl::mouseKeptCentered(bool mouseKeptCentered)
{
    if (m_mouseKeptCentered == mouseKeptCentered) return;
    m_mouseKeptCentered = mouseKeptCentered;

    m_mouseCurrentlyCentered = false;
    m_mouseMoveAccumulator.x = 0;
    m_mouseMoveAccumulator.y = 0;
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
    value_list[1] = EVENT_MASK_FLAGS;
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

void Window::Impl::setupXkb()
{
    // @note All that knowledge for correct setup comes from
    // - https://github.com/xkbcommon/libxkbcommon/blob/master/doc/quick-guide.md
    // - https://xkbcommon.org/doc/current/group__x11.html
    xkb_x11_setup_xkb_extension(m_connection, XKB_X11_MIN_MAJOR_XKB_VERSION, XKB_X11_MIN_MINOR_XKB_VERSION,
                                XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS, nullptr, nullptr, nullptr, nullptr);

    // Context
    m_xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (m_xkbContext == nullptr) {
        logger.error("crater.xcb.window") << "Could not setup XKB context." << std::endl;
    }

    // Keymap
    auto deviceId = xkb_x11_get_core_keyboard_device_id(m_connection);
    if (deviceId == -1) {
        logger.error("crater.xcb.window") << "Could not get XKB device ID." << std::endl;
    }

    m_xkbKeymap = xkb_x11_keymap_new_from_device(m_xkbContext, m_connection, deviceId, XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (m_xkbKeymap == nullptr) {
        logger.error("crater.xcb.window") << "Could not setup XKB keymap." << std::endl;
    }

    // State
    m_xkbState = xkb_x11_state_new_from_device(m_xkbKeymap, m_connection, deviceId);
    if (m_xkbState == nullptr) {
        logger.error("crater.xcb.window") << "Could not setup XKB state." << std::endl;
    }
}

void Window::Impl::mouseMoveIgnored(bool ignored) const
{
    uint32_t eventMaskValues = EVENT_MASK_FLAGS;
    if (ignored) {
        eventMaskValues ^= XCB_EVENT_MASK_POINTER_MOTION;
    }
    xcb_change_window_attributes(m_connection, m_window, XCB_CW_EVENT_MASK, &eventMaskValues);
}

void Window::Impl::processEvents()
{
    xcb_flush(m_connection);

    xcb_generic_event_t* event;
    while ((event = xcb_poll_for_event(m_connection))) {
        processEvent(*event);
        free(event);
    }

    if (m_mouseKeptCentered && !m_mouseCurrentlyCentered) {
        mouseMoveIgnored(true);
        xcb_warp_pointer(m_connection, XCB_NONE, m_window, 0, 0, 0, 0,
                         m_extent.width / 2, m_extent.height / 2);
        mouseMoveIgnored(false);

        m_mouseCurrentlyCentered = true;
        m_mousePositionToReset = true; // Won't emit delta for the centering
    }
}

void Window::Impl::processEvent(xcb_generic_event_t& windowEvent)
{
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

        // @note To prevent big deltas when moving
        // the mouse the first time, we use this flag.
        if (m_mousePositionToReset) {
            m_mousePosition.x = motionEvent.event_x;
            m_mousePosition.y = motionEvent.event_y;
            m_mousePositionToReset = false;
            return;
        }

        event.mouseMove.x = motionEvent.event_x;
        event.mouseMove.y = motionEvent.event_y;
        event.mouseMove.dx = event.mouseMove.x - m_mousePosition.x;
        event.mouseMove.dy = event.mouseMove.y - m_mousePosition.y;

        m_mouseCurrentlyCentered = false;
        m_mousePosition.x = motionEvent.event_x;
        m_mousePosition.y = motionEvent.event_y;

        pushEvent(event);
        break;
    }

    case XCB_KEY_PRESS: {
        auto keyEvent = reinterpret_cast<xcb_key_press_event_t&>(windowEvent);
        xkb_keycode_t keycode = keyEvent.detail;
        xkb_state_update_key(m_xkbState, keycode, XKB_KEY_DOWN);

        auto keysym = xkb_state_key_get_one_sym(m_xkbState, keycode);
        wchar_t code = xkb_state_key_get_utf32(m_xkbState, keycode);

        WsEvent event;
        event.type = WsEventType::KeyPressed;
        event.key.which = keysymToKey(keysym);
        event.key.code = code;
        pushEvent(event);
    } break;

    case XCB_KEY_RELEASE: {
        auto keyEvent = reinterpret_cast<xcb_key_release_event_t&>(windowEvent);
        xkb_keycode_t keycode = keyEvent.detail;
        xkb_state_update_key(m_xkbState, keycode, XKB_KEY_UP);

        auto keysym = xkb_state_key_get_one_sym(m_xkbState, keycode);
        wchar_t code = xkb_state_key_get_utf32(m_xkbState, keycode);

        WsEvent event;
        event.type = WsEventType::KeyReleased;
        event.key.which = keysymToKey(keysym);
        event.key.code = code;
        pushEvent(event);
    } break;

    default: break;
    }
}
