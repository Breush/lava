#pragma once

#include <lava/crater/VideoMode.hpp>
#include <lava/crater/WindowHandle.hpp>
#include <lava/crater/WindowStyle.hpp>
#include <string>

namespace lava::priv {
    class WindowImpl;
}

namespace lava {
    class Event;

    /// \brief Window that serves as a target for OpenGL rendering
    class Window {
    public:
        /// \brief Default constructor
        ///
        /// This constructor doesn't actually create the window,
        /// use the other constructors or call create() to do so.
        ///

        Window();

        /// \brief Construct a new window
        ///
        /// This constructor creates the window with the size and pixel
        /// depth defined in \a mode. An optional style can be passed to
        /// customize the look and behavior of the window (borders,
        /// title bar, resizable, closable, ...). If \a style contains
        /// Style::Fullscreen, then \a mode must be a valid video mode.
        ///
        /// \param mode     Video mode to use (defines the width, height and depth of the rendering area of the window)
        /// \param title    Title of the window
        /// \param style    %Window style, a bitwise OR combination of lava::Style enumerators
        Window(VideoMode mode, const std::string& title, uint32_t style = Style::Default);

        /// Closes the window and frees all the resources attached to it.
        virtual ~Window();

        /// \brief Create (or recreate) the window
        ///
        /// If the window was already created, it closes it first.
        /// If \a style contains Style::Fullscreen, then \a mode
        /// must be a valid video mode.
        ///
        /// The fourth parameter is an optional structure specifying
        /// advanced OpenGL context settings such as antialiasing,
        /// depth-buffer bits, etc.
        ///
        /// \param mode     Video mode to use (defines the width, height and depth of the rendering area of the window)
        /// \param title    Title of the window
        /// \param style    %Window style, a bitwise OR combination of lava::Style enumerators
        void create(VideoMode mode, const std::string& title, uint32_t style = Style::Default);

        /// \brief Close the window and destroy all the attached resources
        ///
        /// After calling this function, the lava::Window instance remains
        /// valid and you can call create() to recreate the window.
        /// All other functions such as pollEvent() or display() will
        /// still work (i.e. you don't have to test isOpen() every time),
        /// and will have no effect on closed windows.
        void close();

        /// \brief Tell whether or not the window is open
        ///
        /// This function returns whether or not the window exists.
        /// Note that a hidden window (setVisible(false)) is open
        /// (therefore this function would return true).
        ///
        /// \return True if the window is open, false if it has been closed
        bool isOpen() const;

        /// \brief Pop the event on top of the event queue, if any, and return it
        ///
        /// This function is not blocking: if there's no pending event then
        /// it will return false and leave \a event unmodified.
        /// Note that more than one event may be present in the event queue,
        /// thus you should always call this function in a loop
        /// to make sure that you process every pending event.
        /// \code
        /// lava::Event event;
        /// while (window.pollEvent(event))
        /// {
        ///    // process event...
        /// }
        /// \endcode
        ///
        /// \param event Event to be returned
        ///
        /// \return True if an event was returned, or false if the event queue was empty
        ///
        /// \see waitEvent
        bool pollEvent(Event& event);

        /// \brief Wait for an event and return it
        ///
        /// This function is blocking: if there's no pending event then
        /// it will wait until an event is received.
        /// After this function returns (and no error occurred),
        /// the \a event object is always valid and filled properly.
        /// This function is typically used when you have a thread that
        /// is dedicated to events handling: you want to make this thread
        /// sleep as long as no new event is received.
        /// \code
        /// lava::Event event;
        /// if (window.waitEvent(event))
        /// {
        ///    // process event...
        /// }
        /// \endcode
        ///
        /// \param event Event to be returned
        ///
        /// \return False if any error occurred
        ///
        /// \see pollEvent
        bool waitEvent(Event& event);

        /// \brief Get the OS-specific handle of the window
        ///
        /// The type of the returned handle is lava::WindowHandle,
        /// which is a typedef to the handle type defined by the OS.
        /// You shouldn't need to use this function, unless you have
        /// very specific stuff to implement that lava doesn't support,
        /// or implement a temporary workaround until a bug is fixed.
        ///
        /// \return System handle of the window
        WindowHandle getSystemHandle() const;

        VideoMode videoMode() const;

    private:
        priv::WindowImpl* m_impl = nullptr; ///< Platform-specific implementation of the window
    };
}
