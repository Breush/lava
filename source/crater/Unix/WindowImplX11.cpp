
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2017 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

// Headers
#include <lava/crater/WindowStyle.hpp> // important to be included first (conflict with None)
#include "./WindowImplX11.hpp"
#include "./Display.hpp"
#include <lava/chamber/Utf.hpp>
#include <lava/chamber/Err.hpp>
#include <lava/chamber/Mutex.hpp>
#include <lava/chamber/Lock.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <fcntl.h>
#include <algorithm>
#include <vector>
#include <string>
#include <cstring>

#include "./GlxContext.hpp"
typedef lava::priv::GlxContext ContextType;

// Private data
namespace
{
    inline xcb_intern_atom_reply_t* internAtomHelper(xcb_connection_t *conn, bool only_if_exists, const char *str)
    {
        xcb_intern_atom_cookie_t cookie = xcb_intern_atom(conn, only_if_exists, strlen(str), str);
        return xcb_intern_atom_reply(conn, cookie, NULL);
    }

    /*
    lava::priv::WindowImplX11*              fullscreenWindow = nullptr;
    std::vector<lava::priv::WindowImplX11*> allWindows;
    lava::Mutex                             allWindowsMutex;
    lava::String                            windowManagerName;

    static const unsigned long            eventMask = FocusChangeMask      | ButtonPressMask     |
                                                      ButtonReleaseMask    | ButtonMotionMask    |
                                                      PointerMotionMask    | KeyPressMask        |
                                                      KeyReleaseMask       | StructureNotifyMask |
                                                      EnterWindowMask      | LeaveWindowMask     |
                                                      VisibilityChangeMask | PropertyChangeMask;

    static const unsigned int             maxTrialsCount = 5;

    // Filter the events received by windows (only allow those matching a specific window)
    Bool checkEvent(::Display*, XEvent* event, XPointer userData)
    {
        // Just check if the event matches the window
        return event->xany.window == reinterpret_cast< ::Window >(userData);
    }

    // Find the name of the current executable
    std::string findExecutableName()
    {
        // We use /proc/self/cmdline to get the command line
        // the user used to invoke this instance of the application
        int file = ::open("/proc/self/cmdline", O_RDONLY | O_NONBLOCK);

        if (file < 0)
            return "sfml";

        std::vector<char> buffer(256, 0);
        std::size_t offset = 0;
        ssize_t result = 0;

        while ((result = read(file, &buffer[offset], 256)) > 0)
        {
            buffer.resize(buffer.size() + result, 0);
            offset += result;
        }

        ::close(file);

        if (offset)
        {
            buffer[offset] = 0;

            // Remove the path to keep the executable name only
            return basename(&buffer[0]);
        }

        // Default fallback name
        return "sfml";
    }
    */
}


using namespace lava;
using namespace lava::priv;

WindowImplX11::WindowImplX11(VideoMode mode, const String& title, unsigned long style, const ContextSettings& settings)
{
    initXcbConnection();
    setupWindow(mode);
}


WindowImplX11::~WindowImplX11()
{
}


void WindowImplX11::initXcbConnection()
{
    const xcb_setup_t *setup;
    xcb_screen_iterator_t iter;

    int scr;
    m_connection = xcb_connect(nullptr, &scr);
    if (m_connection == nullptr) {
        err() << "Could not find a XCB connection.\n" << std::endl;
        exit(1);
    }

    setup = xcb_get_setup(m_connection);
    iter = xcb_setup_roots_iterator(setup);
    while (scr-- > 0)
        xcb_screen_next(&iter);
    m_screen = iter.data;
}


void WindowImplX11::setupWindow(VideoMode mode)
{
    uint32_t value_mask, value_list[32];

    m_window = xcb_generate_id(m_connection);

    value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    value_list[0] = m_screen->black_pixel;
    value_list[1] =
        XCB_EVENT_MASK_KEY_RELEASE |
        XCB_EVENT_MASK_KEY_PRESS |
        XCB_EVENT_MASK_EXPOSURE |
        XCB_EVENT_MASK_STRUCTURE_NOTIFY |
        XCB_EVENT_MASK_POINTER_MOTION |
        XCB_EVENT_MASK_BUTTON_PRESS |
        XCB_EVENT_MASK_BUTTON_RELEASE;

    /*if (settings.fullscreen)
    {
        width = destWidth = m_screen->width_in_pixels;
        height = destHeight = m_screen->height_in_pixels;
    }*/

    xcb_create_window(m_connection,
        XCB_COPY_FROM_PARENT,
        m_window, m_screen->root,
        0, 0, mode.width, mode.height, 0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        m_screen->root_visual,
        value_mask, value_list);

    // Enable window destroyed notifications
    auto reply = internAtomHelper(m_connection, true, "WM_PROTOCOLS");
    m_atomWmDeleteWindow = internAtomHelper(m_connection, false, "WM_DELETE_WINDOW");

    xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE,
        m_window, reply->atom, 4, 32, 1,
        &m_atomWmDeleteWindow->atom);

    std::string windowTitle = "What a nice title!";
    xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE,
        m_window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
        windowTitle.size(), windowTitle.c_str());

    free(reply);

    /*if (settings.fullscreen)
    {
        xcb_intern_atom_reply_t *atom_wm_state = internAtomHelper(m_connection, false, "_NET_WM_STATE");
        xcb_intern_atom_reply_t *atom_wm_fullscreen = internAtomHelper(m_connection, false, "_NET_WM_STATE_FULLSCREEN");
        xcb_change_property(m_connection,
                XCB_PROP_MODE_REPLACE,
                m_window, atom_wm_state->atom,
                XCB_ATOM_ATOM, 32, 1,
                &(atom_wm_fullscreen->atom));
        free(atom_wm_fullscreen);
        free(atom_wm_state);
    }*/

    xcb_map_window(m_connection, m_window);
}


WindowHandle WindowImplX11::getSystemHandle() const
{
    return { m_connection, m_window };
}


void WindowImplX11::processEvents()
{
    xcb_generic_event_t* event;
    while ((event = xcb_poll_for_event(m_connection))) {
        processEvent(*event);
        free(event);
    }
}


Vector2i WindowImplX11::getPosition() const
{
    // Not implemented yet
    return Vector2i(0, 0);
}


void WindowImplX11::setPosition(const Vector2i& position)
{
    // Not implemented yet
}


Vector2u WindowImplX11::getSize() const
{
    // Not implemented yet
    return Vector2u(0, 0);
}


void WindowImplX11::setSize(const Vector2u& size)
{
    // Not implemented yet
}


void WindowImplX11::setTitle(const String& title)
{
    // Not implemented yet
}


void WindowImplX11::setIcon(unsigned int width, unsigned int height, const uint8_t* pixels)
{
    // Not implemented yet
}


void WindowImplX11::setVisible(bool visible)
{
    // Not implemented yet
}


void WindowImplX11::setMouseCursorVisible(bool visible)
{
    // Not implemented yet
}


void WindowImplX11::setMouseCursorGrabbed(bool grabbed)
{
    // Not implemented yet
}


void WindowImplX11::setKeyRepeatEnabled(bool enabled)
{
    // Not implemented yet
}


void WindowImplX11::requestFocus()
{
    // Not implemented yet
}


bool WindowImplX11::hasFocus() const
{
    // Not implemented yet
    return true;
}


void WindowImplX11::grabFocus()
{
    // Not implemented yet
}


void WindowImplX11::setVideoMode(const VideoMode& mode)
{
    // Not implemented yet
}


void WindowImplX11::resetVideoMode()
{
    // Not implemented yet
}


void WindowImplX11::switchToFullscreen()
{
    // Not implemented yet
}


void WindowImplX11::initialize()
{
    // Not implemented yet
}


void WindowImplX11::createHiddenCursor()
{
    // Not implemented yet
}


void WindowImplX11::cleanup()
{
    resetVideoMode();
    setMouseCursorVisible(true);
}


bool WindowImplX11::processEvent(xcb_generic_event_t& windowEvent)
{
    switch (windowEvent.response_type & 0x7f) {
    case XCB_DESTROY_NOTIFY:
        cleanup();
        break;

    case XCB_CLIENT_MESSAGE:
        auto messageEvent = reinterpret_cast<xcb_client_message_event_t&>(windowEvent);
        if (messageEvent.data.data32[0] == m_atomWmDeleteWindow->atom) {
            Event event;
            event.type = Event::Closed;
            pushEvent(event);
        }
        break;

        // Gain focus event
        /*case FocusIn:
        {
            // Update the input context
            if (m_inputContext)
                XSetICFocus(m_inputContext);

            // Grab cursor
            if (m_cursorGrabbed)
            {
                // Try multiple times to grab the cursor
                for (unsigned int trial = 0; trial < maxTrialsCount; ++trial)
                {
                    int result = XGrabPointer(m_display, m_window, True, None, GrabModeAsync, GrabModeAsync, m_window, None, CurrentTime);

                    if (result == GrabSuccess)
                    {
                        m_cursorGrabbed = true;
                        break;
                    }

                    // The cursor grab failed, trying again after a small sleep
                    // lava::sleep(lava::milliseconds(50));
                }

                if (!m_cursorGrabbed)
                    err() << "Failed to grab mouse cursor" << std::endl;
            }

            Event event;
            event.type = Event::GainedFocus;
            pushEvent(event);

            // If the window has been previously marked urgent (notification) as a result of a focus request, undo that
            XWMHints* hints = XGetWMHints(m_display, m_window);
            if (hints != nullptr)
            {
                // Remove urgency (notification) flag from hints
                hints->flags &= ~XUrgencyHint;
                XSetWMHints(m_display, m_window, hints);
                XFree(hints);
            }

            break;
        }

        // Lost focus event
        case FocusOut:
        {
            // Update the input context
            if (m_inputContext)
                XUnsetICFocus(m_inputContext);

            // Release cursor
            if (m_cursorGrabbed)
                XUngrabPointer(m_display, CurrentTime);

            Event event;
            event.type = Event::LostFocus;
            pushEvent(event);
            break;
        }

        // Resize event
        case ConfigureNotify:
        {
            // ConfigureNotify can be triggered for other reasons, check if the size has actually changed
            if ((windowEvent.xconfigure.width != m_previousSize.x) || (windowEvent.xconfigure.height != m_previousSize.y))
            {
                Event event;
                event.type        = Event::Resized;
                event.size.width  = windowEvent.xconfigure.width;
                event.size.height = windowEvent.xconfigure.height;
                pushEvent(event);

                m_previousSize.x = windowEvent.xconfigure.width;
                m_previousSize.y = windowEvent.xconfigure.height;
            }
            break;
        }

        // Close event
        case ClientMessage:
        {
            static Atom wmProtocols = getAtom("WM_PROTOCOLS");

            // Handle window manager protocol messages we support
            if (windowEvent.xclient.message_type == wmProtocols)
            {
                static Atom wmDeleteWindow = getAtom("WM_DELETE_WINDOW");
                static Atom netWmPing = ewmhSupported() ? getAtom("_NET_WM_PING", true) : None;

                if ((windowEvent.xclient.format == 32) && (windowEvent.xclient.data.l[0]) == static_cast<long>(wmDeleteWindow))
                {
                    // Handle the WM_DELETE_WINDOW message
                    Event event;
                    event.type = Event::Closed;
                    pushEvent(event);
                }
                else if (netWmPing && (windowEvent.xclient.format == 32) && (windowEvent.xclient.data.l[0]) == static_cast<long>(netWmPing))
                {
                    // Handle the _NET_WM_PING message, send pong back to WM to show that we are responsive
                    windowEvent.xclient.window = DefaultRootWindow(m_display);

                    XSendEvent(m_display, DefaultRootWindow(m_display), False, SubstructureNotifyMask | SubstructureRedirectMask, &windowEvent);
                }
            }
            break;
        }*/

        // Key down event
        /*case KeyPress:
        {
            Keyboard::Key key = Keyboard::Unknown;

            // Try each KeySym index (modifier group) until we get a match
            for (int i = 0; i < 4; ++i)
            {
                // Get the SFML keyboard code from the keysym of the key that has been pressed
                key = keysymToSF(XLookupKeysym(&windowEvent.xkey, i));

                if (key != Keyboard::Unknown)
                    break;
            }

            // Fill the event parameters
            // TODO: if modifiers are wrong, use XGetModifierMapping to retrieve the actual modifiers mapping
            Event event;
            event.type        = Event::KeyPressed;
            event.key.code    = key;
            event.key.alt     = windowEvent.xkey.state & Mod1Mask;
            event.key.control = windowEvent.xkey.state & ControlMask;
            event.key.shift   = windowEvent.xkey.state & ShiftMask;
            event.key.system  = windowEvent.xkey.state & Mod4Mask;
            pushEvent(event);

            // Generate a TextEntered event
            if (!XFilterEvent(&windowEvent, None))
            {
                #ifdef X_HAVE_UTF8_STRING
                if (m_inputContext)
                {
                    Status status;
                    Uint8  keyBuffer[16];

                    int length = Xutf8LookupString(
                        m_inputContext,
                        &windowEvent.xkey,
                        reinterpret_cast<char*>(keyBuffer),
                        sizeof(keyBuffer),
                        nullptr,
                        &status
                    );

                    if (length > 0)
                    {
                        Uint32 unicode = 0;
                        Utf8::decode(keyBuffer, keyBuffer + length, unicode, 0);
                        if (unicode != 0)
                        {
                            Event textEvent;
                            textEvent.type         = Event::TextEntered;
                            textEvent.text.unicode = unicode;
                            pushEvent(textEvent);
                        }
                    }
                }
                else
                #endif
                {
                    static XComposeStatus status;
                    char keyBuffer[16];
                    if (XLookupString(&windowEvent.xkey, keyBuffer, sizeof(keyBuffer), nullptr, &status))
                    {
                        Event textEvent;
                        textEvent.type         = Event::TextEntered;
                        textEvent.text.unicode = static_cast<Uint32>(keyBuffer[0]);
                        pushEvent(textEvent);
                    }
                }
            }

            updateLastInputTime(windowEvent.xkey.time);

            break;
        }

        // Key up event
        case KeyRelease:
        {
            Keyboard::Key key = Keyboard::Unknown;

            // Try each KeySym index (modifier group) until we get a match
            for (int i = 0; i < 4; ++i)
            {
                // Get the SFML keyboard code from the keysym of the key that has been released
                key = keysymToSF(XLookupKeysym(&windowEvent.xkey, i));

                if (key != Keyboard::Unknown)
                    break;
            }

            // Fill the event parameters
            Event event;
            event.type        = Event::KeyReleased;
            event.key.code    = key;
            event.key.alt     = windowEvent.xkey.state & Mod1Mask;
            event.key.control = windowEvent.xkey.state & ControlMask;
            event.key.shift   = windowEvent.xkey.state & ShiftMask;
            event.key.system  = windowEvent.xkey.state & Mod4Mask;
            pushEvent(event);

            break;
        }

        // Mouse button pressed
        case ButtonPress:
        {
            // XXX: Why button 8 and 9?
            // Because 4 and 5 are the vertical wheel and 6 and 7 are horizontal wheel ;)
            unsigned int button = windowEvent.xbutton.button;
            if ((button == Button1) ||
                (button == Button2) ||
                (button == Button3) ||
                (button == 8) ||
                (button == 9))
            {
                Event event;
                event.type          = Event::MouseButtonPressed;
                event.mouseButton.x = windowEvent.xbutton.x;
                event.mouseButton.y = windowEvent.xbutton.y;
                switch(button)
                {
                    case Button1: event.mouseButton.button = Mouse::Left;     break;
                    case Button2: event.mouseButton.button = Mouse::Middle;   break;
                    case Button3: event.mouseButton.button = Mouse::Right;    break;
                    case 8:       event.mouseButton.button = Mouse::XButton1; break;
                    case 9:       event.mouseButton.button = Mouse::XButton2; break;
                }
                pushEvent(event);
            }

            updateLastInputTime(windowEvent.xbutton.time);

            break;
        }

        // Mouse button released
        case ButtonRelease:
        {
            unsigned int button = windowEvent.xbutton.button;
            if ((button == Button1) ||
                (button == Button2) ||
                (button == Button3) ||
                (button == 8) ||
                (button == 9))
            {
                Event event;
                event.type          = Event::MouseButtonReleased;
                event.mouseButton.x = windowEvent.xbutton.x;
                event.mouseButton.y = windowEvent.xbutton.y;
                switch(button)
                {
                    case Button1: event.mouseButton.button = Mouse::Left;     break;
                    case Button2: event.mouseButton.button = Mouse::Middle;   break;
                    case Button3: event.mouseButton.button = Mouse::Right;    break;
                    case 8:       event.mouseButton.button = Mouse::XButton1; break;
                    case 9:       event.mouseButton.button = Mouse::XButton2; break;
                }
                pushEvent(event);
            }
            else if ((button == Button4) || (button == Button5))
            {
                Event event;

                event.type             = Event::MouseWheelMoved;
                event.mouseWheel.delta = (button == Button4) ? 1 : -1;
                event.mouseWheel.x     = windowEvent.xbutton.x;
                event.mouseWheel.y     = windowEvent.xbutton.y;
                pushEvent(event);

                event.type                   = Event::MouseWheelScrolled;
                event.mouseWheelScroll.wheel = Mouse::VerticalWheel;
                event.mouseWheelScroll.delta = (button == Button4) ? 1 : -1;
                event.mouseWheelScroll.x     = windowEvent.xbutton.x;
                event.mouseWheelScroll.y     = windowEvent.xbutton.y;
                pushEvent(event);
            }
            else if ((button == 6) || (button == 7))
            {
                Event event;
                event.type                   = Event::MouseWheelScrolled;
                event.mouseWheelScroll.wheel = Mouse::HorizontalWheel;
                event.mouseWheelScroll.delta = (button == 6) ? 1 : -1;
                event.mouseWheelScroll.x     = windowEvent.xbutton.x;
                event.mouseWheelScroll.y     = windowEvent.xbutton.y;
                pushEvent(event);
            }
            break;
        }

        // Mouse moved
        case MotionNotify:
        {
            Event event;
            event.type        = Event::MouseMoved;
            event.mouseMove.x = windowEvent.xmotion.x;
            event.mouseMove.y = windowEvent.xmotion.y;
            pushEvent(event);
            break;
        }

        // Mouse entered
        case EnterNotify:
        {
            if (windowEvent.xcrossing.mode == NotifyNormal)
            {
                Event event;
                event.type = Event::MouseEntered;
                pushEvent(event);
            }
            break;
        }

        // Mouse left
        case LeaveNotify:
        {
            if (windowEvent.xcrossing.mode == NotifyNormal)
            {
                Event event;
                event.type = Event::MouseLeft;
                pushEvent(event);
            }
            break;
        }*/

        // Window unmapped
        /*case UnmapNotify:
        {
            if (windowEvent.xunmap.window == m_window)
                m_windowMapped = false;

            break;
        }

        // Window visibility change
        case VisibilityNotify:
        {
            // We prefer using VisibilityNotify over MapNotify because
            // some window managers like awesome don't internally flag a
            // window as viewable even after it is mapped but before it
            // is visible leading to certain function calls failing with
            // an unviewable error if called before VisibilityNotify arrives

            // Empirical testing on most widely used window managers shows
            // that mapping a window will always lead to a VisibilityNotify
            // event that is not VisibilityFullyObscured
            if (windowEvent.xvisibility.window == m_window)
            {
                if (windowEvent.xvisibility.state != VisibilityFullyObscured)
                    m_windowMapped = true;
            }

            break;
        }

        // Window property change
        case PropertyNotify:
        {
            if (!m_lastInputTime)
                m_lastInputTime = windowEvent.xproperty.time;

            break;
        }*/
    }

    return true;
}
