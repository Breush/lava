#pragma once

#include <lava/magma/render-targets/window-render-target.hpp>

#include "./i-render-target-impl.hpp"

#include <lava/core/macros.hpp>
#include <lava/crater/window.hpp>
#include <lava/magma/render-engine.hpp>

#include "../holders/swapchain-holder.hpp"
#include "../wrappers.hpp"

namespace lava::magma {
    class WindowRenderTarget::Impl final : public IRenderTarget::Impl {
    public:
        Impl(RenderEngine& engine, WsHandle handle, const Extent2d& extent);

        // IRenderTarget::Impl
        void init(uint32_t id) override final;
        void prepare() override final;
        void draw(vk::Semaphore renderFinishedSemaphore) const override final;

        uint32_t id() const { return m_id; }
        const vulkan::SwapchainHolder& swapchainHolder() const override final { return m_swapchainHolder; }
        vk::SurfaceKHR surface() const override final { return m_surface; }

        // WindowRenderTarget
        inline Extent2d extent() const { return {m_windowExtent.width, m_windowExtent.height}; }
        void extent(const Extent2d& extent);
        WsHandle handle() const { return m_handle; }

    protected:
        // Internal
        void initSurface();
        void initSwapchain();
        void recreateSwapchain();

    private:
        // References
        RenderEngine::Impl& m_engine;
        uint32_t m_id = -1u;
        WsHandle m_handle;

        // Resources
        vulkan::SurfaceKHR m_surface;
        vulkan::SwapchainHolder m_swapchainHolder;
        vk::Extent2D m_windowExtent;
    };
}
