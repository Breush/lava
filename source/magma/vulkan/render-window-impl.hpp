#pragma once

#include <lava/magma/render-engine.hpp>
#include <lava/magma/render-window.hpp>

#include <lava/chamber/macros.hpp>
#include <lava/crater/window.hpp>
#include <vulkan/vulkan.hpp>

#include "./render-target-data.hpp"
#include "./wrappers.hpp"

namespace lava::magma {
    class RenderWindow::Impl {
    public:
        Impl(RenderEngine& engine, crater::VideoMode mode, const std::string& title);

        // IRenderTarget
        void init(IRenderTarget::UserData data);
        void prepare();
        void draw(IRenderTarget::UserData data) const;
        IRenderTarget::UserData data() const { return &m_renderTargetData; }

        // crater::Window forwarding
        bool pollEvent(crater::Event& event);
        void close();

        crater::WindowHandle windowHandle() const;
        const crater::VideoMode& videoMode() const;
        void videoMode(const crater::VideoMode& mode);
        bool opened() const;

    protected:
        // Internal
        void initSurface();
        void initSwapchain();
        void recreateSwapchain();

    private:
        // References
        RenderEngine::Impl& m_engine;
        uint32_t m_id = -1u;

        // Resources
        vulkan::SurfaceKHR m_surface;
        vulkan::SwapchainHolder m_swapchainHolder;
        DataRenderTarget m_renderTargetData;
        crater::Window m_window;
        vk::Extent2D m_windowExtent;
    };
}
