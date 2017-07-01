#pragma once

#include <lava/crater/window.hpp>
#include <lava/magma/render-engine.hpp>
#include <lava/magma/render-window.hpp>
#include <vulkan/vulkan.hpp>

namespace lava::vulkan {
    class Swapchain;
}

namespace lava {
    class RenderWindow::Impl {
    public:
        Impl(crater::VideoMode mode, const std::string& title);

        // IRenderTarget
        void init(RenderEngine& engine);
        void draw() const;
        void refresh();

        bool pollEvent(crater::Event& event);
        void close();

        crater::WindowHandle windowHandle() const;
        crater::VideoMode videoMode() const;
        void videoMode(const crater::VideoMode& mode);
        bool opened() const;

    private:
        crater::Window m_window;
        VkExtent2D m_windowExtent;

        RenderEngine::Impl* m_engine = nullptr;
        vulkan::Swapchain* m_swapchain = nullptr;
    };
}
