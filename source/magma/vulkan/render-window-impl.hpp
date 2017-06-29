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
        Impl(VideoMode mode, const std::string& title);

        // IRenderTarget
        void init(RenderEngine& engine);
        void draw() const;
        void refresh();

        bool pollEvent(Event& event);
        void close();

        WindowHandle systemHandle() const;
        VideoMode videoMode() const;
        void videoMode(const VideoMode& mode);
        bool opened() const;

    private:
        Window m_window;
        VkExtent2D m_windowExtent;

        RenderEngine::Impl* m_engine = nullptr;
        vulkan::Swapchain* m_swapchain = nullptr;
    };
}
