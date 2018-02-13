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

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
