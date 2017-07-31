#pragma once

#include <lava/crater/window.hpp>
#include <lava/magma/render-engine.hpp>
#include <lava/magma/render-window.hpp>
#include <vulkan/vulkan.hpp>

namespace lava::magma::vulkan {
    class Swapchain;
}

namespace lava::magma {
    class RenderWindow::Impl {
    public:
        Impl(RenderEngine& engine, crater::VideoMode mode, const std::string& title);

        // IRenderTarget
        void draw() const;
        void refresh();

        // crater::Window forwarding
        bool pollEvent(crater::Event& event);
        void close();

        crater::WindowHandle windowHandle() const;
        crater::VideoMode videoMode() const;
        void videoMode(const crater::VideoMode& mode);
        bool opened() const;

    private:
        // References
        RenderEngine::Impl& m_engine;

        // Resources
        crater::Window m_window;
        VkExtent2D m_windowExtent;
    };
}
