#pragma once

#include <lava/core/ws-event.hpp>
#include <lava/core/ws-handle.hpp>
#include <lava/crater/video-mode.hpp>

#include <optional>
#include <string>

namespace lava::crater {
    class Window {
    public:
        Window();
        Window(VideoMode mode, const std::string& title);
        virtual ~Window();

        void create(VideoMode mode, const std::string& title);
        std::optional<WsEvent> pollEvent();
        void close();

        bool opened() const;
        WsHandle handle() const;
        const VideoMode& videoMode() const;
        Extent2d extent() const;

        /// Whether the window should be fullscreen (with no decorations).
        bool fullscreen() const;
        void fullscreen(bool fullscreen);

        /// Hide the mouse cursor when inside the window.
        bool mouseHidden() const;
        void mouseHidden(bool mouseHidden);

        /// Keep the mouse cursor centered in the window.
        /// Mouse positions become almost useless, use deltas instead.
        bool mouseKeptCentered() const;
        void mouseKeptCentered(bool mouseKeptCentered);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
