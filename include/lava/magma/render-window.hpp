#pragma once

#include <lava/chamber/macros.hpp>
#include <lava/crater/event.hpp>
#include <lava/crater/video-mode.hpp>
#include <lava/crater/window-handle.hpp>
#include <lava/magma/interfaces/render-target.hpp>

#include <string>

namespace lava::magma {
    /**
     * A window that can be rendered to.
     */
    class RenderWindow final : public IRenderTarget {
    public:
        RenderWindow(RenderEngine& engine, crater::VideoMode mode, const std::string& title);
        ~RenderWindow();

        // IRenderTarget
        void init(UserData data) override final;
        void prepare() override final;
        void draw(UserData data) const override final;
        UserData data() override final;

        bool pollEvent(crater::Event& event);
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
