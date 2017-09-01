#pragma once

#include <lava/magma/render-targets/render-window.hpp>

#include "./i-render-target-impl.hpp"

#include <lava/chamber/macros.hpp>
#include <lava/crater/window.hpp>
#include <lava/magma/render-engine.hpp>

#include "../holders/swapchain-holder.hpp"
#include "../wrappers.hpp"

namespace lava::magma {
    class RenderWindow::Impl final : public IRenderTarget::Impl {
    public:
        Impl(RenderEngine& engine, crater::VideoMode mode, const std::string& title);

        // IRenderTarget::Impl
        void init(uint32_t id) override final;
        void prepare() override final;
        void draw(vk::Semaphore renderFinishedSemaphore) const override final;

        uint32_t id() const { return m_id; }
        const vulkan::SwapchainHolder& swapchainHolder() const override final { return m_swapchainHolder; }
        vk::SurfaceKHR surface() const override final { return m_surface; }

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
        crater::Window m_window;
        vk::Extent2D m_windowExtent;
    };
}
