#pragma once

#include <lava/crater/video-mode.hpp>
#include <lava/crater/window-handle.hpp>
#include <string>

namespace lava {
    class Event;
}

namespace lava {
    class Window {
    public:
        Window();
        Window(VideoMode mode, const std::string& title);
        virtual ~Window();

        void create(VideoMode mode, const std::string& title);
        bool pollEvent(Event& event);
        void close();

        bool opened() const;
        WindowHandle windowHandle() const;
        VideoMode videoMode() const;

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
