#pragma once

#include <lava/crater/video-mode.hpp>

namespace lava::crater {
    /**
     * Will be inherited by platform-specific implementations.
     */
    class IWindowImpl {
    public:
        IWindowImpl() = default;
        IWindowImpl(const VideoMode& videoMode);
        virtual ~IWindowImpl() = default;

        std::optional<WsEvent> popEvent();

        virtual WsHandle handle() const = 0;
        virtual bool fullscreen() const = 0;
        virtual void fullscreen(bool fullscreen) = 0;
         /* @todo Missing implementations for DWM/Wayland. Could be pure virtual otherwise. */
        virtual bool mouseHidden() const { return false; }
        virtual void mouseHidden(bool /* mouseHidden */) {}
        virtual bool mouseKeptCentered() const { return false; }
        virtual void mouseKeptCentered(bool /* mouseKeptCentered */) {}
        const VideoMode& videoMode() const { return m_videoMode; }

    protected:
        void pushEvent(const WsEvent& event);

        /**
         * Add all events from the system.
         */
        virtual void processEvents() = 0;

    private:
        std::queue<WsEvent> m_events;
        VideoMode m_videoMode;
    };
}
