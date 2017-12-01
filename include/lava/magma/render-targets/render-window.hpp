#pragma once

#include <lava/magma/render-targets/i-render-target.hpp>

#include <lava/chamber/macros.hpp>
#include <lava/crater/event.hpp>
#include <lava/crater/video-mode.hpp>
#include <lava/crater/window-handle.hpp>

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

        std::optional<crater::Event> pollEvent();
        void close();

        crater::WindowHandle windowHandle() const;
        const crater::VideoMode& videoMode() const;
        void videoMode(const crater::VideoMode& mode);
        bool opened() const;

    private:
        class Impl;
        Impl* m_impl = nullptr;
    };
}
