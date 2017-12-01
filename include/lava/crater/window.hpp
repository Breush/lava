#pragma once

#include <lava/crater/video-mode.hpp>
#include <lava/crater/window-handle.hpp>

#include <optional>
#include <string>

namespace lava::crater {
    class Event;
}

namespace lava::crater {
    class Window {
    public:
        Window();
        Window(VideoMode mode, const std::string& title);
        virtual ~Window();

        void create(VideoMode mode, const std::string& title);
        std::optional<Event> pollEvent();
        void close();

        bool opened() const;
        WindowHandle windowHandle() const;
        const VideoMode& videoMode() const;

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
