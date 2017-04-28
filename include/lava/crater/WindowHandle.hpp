////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////

#ifndef SFML_WINDOWHANDLE_HPP
#define SFML_WINDOWHANDLE_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <lava/config.hpp>

// Forward declaration
struct xcb_connection_t;

namespace lava
{

    // XCB handle
    typedef struct {
        xcb_connection_t* connection;
        uint32_t window;
    } WindowHandle;

} // namespace lava


#endif // SFML_WINDOWHANDLE_HPP

////////////////////////////////////////////////////////////
/// \typedef lava::WindowHandle
/// \ingroup window
///
/// Define a low-level window handle type, specific to
/// each platform.
///
/// Platform        | Type
/// ----------------|------------------------------------------------------------
/// Windows         | \p HWND
/// Linux/FreeBSD   | \p %Window
/// Mac OS X        | either \p NSWindow* or \p NSView*, disguised as \p void*
/// iOS             | \p UIWindow*
/// Android         | \p ANativeWindow*
///
/// \par Mac OS X Specification
///
/// On Mac OS X, a lava::Window can be created either from an
/// existing \p NSWindow* or an \p NSView*. When the window
/// is created from a window, SFML will use its content view
/// as the OpenGL area. lava::Window::getSystemHandle() will
/// return the handle that was used to create the window,
/// which is a \p NSWindow* by default.
///
////////////////////////////////////////////////////////////
