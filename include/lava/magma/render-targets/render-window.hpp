#pragma once

#include <lava/magma/render-targets/i-render-target.hpp>

#include <lava/chamber/macros.hpp>
#include <lava/core/ws-event.hpp>
#include <lava/core/ws-handle.hpp>

// @fixme These crater internals should not be exposed in magma public headers!
#include <lava/crater/video-mode.hpp>

#include <optional>
#include <string>

namespace lava::magma {
    class RenderEngine;
}

namespace lava::magma {
    /**
     * A window that can be rendered to.
     */
    class RenderWindow final : public IRenderTarget {
    public:
        RenderWindow(RenderEngine& engine, crater::VideoMode mode, const std::string& title);
        ~RenderWindow();

        // IRenderTarget
        IRenderTarget::Impl& interfaceImpl() override final;

        std::optional<WsEvent> pollEvent();
        void close();

        WsHandle handle() const;
        const crater::VideoMode& videoMode() const;
        void videoMode(const crater::VideoMode& mode);
        bool opened() const;

    private:
        class Impl;
        Impl* m_impl = nullptr;
    };
}
