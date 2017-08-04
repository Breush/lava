#pragma once

#include <lava/magma/render-engine.hpp>
#include <lava/magma/render-window.hpp>

#include <lava/chamber/macros.hpp>
#include <lava/crater/window.hpp>
#include <vulkan/vulkan.hpp>

#include "./render-target-data.hpp"
#include "./swapchain.hpp"
#include "./wrappers.hpp"

namespace lava::magma::vulkan {
    class Swapchain;
}

namespace lava::magma {
    class RenderWindow::Impl {
    public:
        Impl(RenderEngine& engine, crater::VideoMode mode, const std::string& title);

        // IRenderTarget
        void init();
        void prepare();
        void draw(IRenderTarget::UserData data) const;
        void refresh();
        IRenderTarget::UserData data() const { return &m_renderTargetData; }

        // crater::Window forwarding
        bool pollEvent(crater::Event& event);
        void close();

        crater::WindowHandle windowHandle() const;
        crater::VideoMode videoMode() const;
        void videoMode(const crater::VideoMode& mode);
        bool opened() const;

    protected:
        // Internal
        void initSurface();
        void initSwapchain();

    private:
        // References
        RenderEngine::Impl& m_engine;

        // Resources
        $attribute(vulkan::SurfaceKHR, surface);
        $attribute(vulkan::Swapchain, swapchain);
        DataRenderTarget m_renderTargetData;
        crater::Window m_window;
        VkExtent2D m_windowExtent;
    };
}
