#pragma once

#include <lava/chamber/macros.hpp>
#include <lava/crater/Event.hpp>
#include <lava/crater/VideoMode.hpp>
#include <lava/crater/WindowHandle.hpp>
#include <lava/magma/interfaces/render-target.hpp>

#include <string>

namespace lava {
    /**
     * A window that can be rendered to.
     */
    class RenderWindow final : public IRenderTarget {
    public:
        RenderWindow(VideoMode mode, const std::string& title);
        ~RenderWindow();

        // IRenderTarget
        void init(RenderEngine& engine) override final;
        void draw() const override final;
        void refresh() override final;

        bool pollEvent(Event& event);
        void close();

        WindowHandle systemHandle() const;
        VideoMode videoMode() const;
        void videoMode(const VideoMode& mode);
        bool opened() const;

    private:
        class Impl;
        Impl* m_impl = nullptr;
    };
}
