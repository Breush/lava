#include "./window-impl.hpp"

#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <lava/chamber/logger.hpp>
#include <libgen.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <xcb/xcb_keysyms.h>

namespace {
    xcb_key_symbols_t* g_keySymbols = nullptr;

    inline xcb_intern_atom_reply_t* internAtomHelper(xcb_connection_t* conn, bool only_if_exists, const char* str)
    {
        xcb_intern_atom_cookie_t cookie = xcb_intern_atom(conn, only_if_exists, strlen(str), str);
        return xcb_intern_atom_reply(conn, cookie, nullptr);
    }

    // Got list from https://github.com/substack/node-keysym/blob/master/data/keysyms.txt
    lava::crater::input::Key keyEventToKey(xcb_key_press_event_t& keyEvent)
    {
        xcb_keysym_t keySym = xcb_key_press_lookup_keysym(g_keySymbols, &keyEvent, 0);

        switch (keySym) {
        case 'a': return lava::crater::input::Key::A;
        case 'b': return lava::crater::input::Key::B;
        case 'c': return lava::crater::input::Key::C;
        case 'd': return lava::crater::input::Key::D;
        case 'e': return lava::crater::input::Key::E;
        case 'f': return lava::crater::input::Key::F;
        case 'g': return lava::crater::input::Key::G;
        case 'h': return lava::crater::input::Key::H;
        case 'i': return lava::crater::input::Key::I;
        case 'j': return lava::crater::input::Key::J;
        case 'k': return lava::crater::input::Key::K;
        case 'l': return lava::crater::input::Key::L;
        case 'm': return lava::crater::input::Key::M;
        case 'n': return lava::crater::input::Key::N;
        case 'o': return lava::crater::input::Key::O;
        case 'p': return lava::crater::input::Key::P;
        case 'q': return lava::crater::input::Key::Q;
        case 'r': return lava::crater::input::Key::R;
        case 's': return lava::crater::input::Key::S;
        case 't': return lava::crater::input::Key::T;
        case 'u': return lava::crater::input::Key::U;
        case 'v': return lava::crater::input::Key::V;
        case 'w': return lava::crater::input::Key::W;
        case 'x': return lava::crater::input::Key::X;
        case 'y': return lava::crater::input::Key::Y;
        case 'z': return lava::crater::input::Key::Z;
        case 0xff1b: return lava::crater::input::Key::Escape;
        case 0xffbe: return lava::crater::input::Key::F1;
        case 0xffbf: return lava::crater::input::Key::F2;
        case 0xffc0: return lava::crater::input::Key::F3;
        case 0xffc1: return lava::crater::input::Key::F4;
        case 0xffc2: return lava::crater::input::Key::F5;
        case 0xffc3: return lava::crater::input::Key::F6;
        case 0xffc4: return lava::crater::input::Key::F7;
        case 0xffc5: return lava::crater::input::Key::F8;
        case 0xffc6: return lava::crater::input::Key::F9;
        case 0xffc7: return lava::crater::input::Key::F10;
        case 0xffc8: return lava::crater::input::Key::F11;
        case 0xffc9: return lava::crater::input::Key::F12;
        case 0xff51: return lava::crater::input::Key::Left;
        case 0xff52: return lava::crater::input::Key::Up;
        case 0xff53: return lava::crater::input::Key::Right;
        case 0xff54: return lava::crater::input::Key::Down;
        }

        return lava::crater::input::Key::Unknown;
    }
}

using namespace lava::crater;
using namespace lava::chamber;

Window::Impl::Impl(VideoMode mode, const std::string& title)
    : IWindowImpl(mode)
{
    initXcbConnection();
    setupWindow(mode, title);

    // @todo If not allocated?
    // Probably more secure to put that as a class member for now.
    g_keySymbols = xcb_key_symbols_alloc(m_connection);
}

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

    xcb_create_window(m_connection, XCB_COPY_FROM_PARENT, m_window, m_screen->root, 0, 0, mode.width, mode.height, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, m_screen->root_visual, value_mask, value_list);

    // Enable window destroyed notifications
    auto reply = internAtomHelper(m_connection, true, "WM_PROTOCOLS");
    m_atomWmDeleteWindow = internAtomHelper(m_connection, false, "WM_DELETE_WINDOW");

    xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, m_window, reply->atom, 4, 32, 1, &m_atomWmDeleteWindow->atom);

    xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, m_window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, title.size(),
                        title.c_str());

    free(reply);

    xcb_map_window(m_connection, m_window);
}

WindowHandle Window::Impl::windowHandle() const
{
    return {m_connection, m_window};
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
        if (messageEvent.data.data32[0] != m_atomWmDeleteWindow->atom) break;

        Event event;
        event.type = Event::WindowClosed;
        pushEvent(event);
        break;
    }

    case XCB_CONFIGURE_NOTIFY: {
        auto configureEvent = reinterpret_cast<xcb_configure_notify_event_t&>(windowEvent);
        if (m_previousSize.x == configureEvent.width && m_previousSize.y == configureEvent.height) break;

        Event event;
        event.type = Event::Type::WindowResized;
        event.size.width = configureEvent.width;
        event.size.height = configureEvent.height;
        pushEvent(event);

        m_previousSize.x = configureEvent.width;
        m_previousSize.y = configureEvent.height;
        break;
    }

    case XCB_BUTTON_PRESS: {
        auto buttonEvent = reinterpret_cast<xcb_button_press_event_t&>(windowEvent);

        Event event;

        // Classic buttons
        if (buttonEvent.detail <= XCB_BUTTON_INDEX_3) {
            event.type = Event::MouseButtonPressed;
            event.mouseButton.x = buttonEvent.event_x;
            event.mouseButton.y = buttonEvent.event_y;
            if (buttonEvent.detail == XCB_BUTTON_INDEX_1) event.mouseButton.which = input::Button::Left;
            if (buttonEvent.detail == XCB_BUTTON_INDEX_2) event.mouseButton.which = input::Button::Middle;
            if (buttonEvent.detail == XCB_BUTTON_INDEX_3) event.mouseButton.which = input::Button::Right;
        }
        // Mouse wheel buttons
        else if (buttonEvent.detail <= XCB_BUTTON_INDEX_5) {
            event.type = Event::MouseScrolled;
            event.mouseScroll.x = buttonEvent.event_x;
            event.mouseScroll.y = buttonEvent.event_y;
            event.mouseScroll.delta = (buttonEvent.detail == XCB_BUTTON_INDEX_4) ? 1 : -1;
        }
        pushEvent(event);
        break;
    }

    case XCB_BUTTON_RELEASE: {
        auto buttonEvent = reinterpret_cast<xcb_button_release_event_t&>(windowEvent);

        Event event;
        event.type = Event::MouseButtonReleased;
        event.mouseButton.x = buttonEvent.event_x;
        event.mouseButton.y = buttonEvent.event_y;
        if (buttonEvent.detail == XCB_BUTTON_INDEX_1) event.mouseButton.which = input::Button::Left;
        if (buttonEvent.detail == XCB_BUTTON_INDEX_2) event.mouseButton.which = input::Button::Middle;
        if (buttonEvent.detail == XCB_BUTTON_INDEX_3) event.mouseButton.which = input::Button::Right;
        pushEvent(event);
        break;
    }

    case XCB_MOTION_NOTIFY: {
        auto motionEvent = reinterpret_cast<xcb_motion_notify_event_t&>(windowEvent);

        Event event;
        event.type = Event::MouseMoved;
        event.mouseMove.x = motionEvent.event_x;
        event.mouseMove.y = motionEvent.event_y;
        pushEvent(event);
        break;
    }

    case XCB_KEY_PRESS: {
        auto keyEvent = reinterpret_cast<xcb_key_press_event_t&>(windowEvent);

        Event event;
        event.type = Event::KeyPressed;
        event.key.which = keyEventToKey(keyEvent);
        pushEvent(event);
    } break;

    case XCB_KEY_RELEASE: {
        auto keyEvent = reinterpret_cast<xcb_key_release_event_t&>(windowEvent);

        Event event;
        event.type = Event::KeyReleased;
        event.key.which = keyEventToKey(keyEvent);
        pushEvent(event);
    } break;

    default: break;
    }

    return true;
}
