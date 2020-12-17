#pragma once

#include <lava/magma/render-targets/window-render-target.hpp>

#include "./i-render-target-impl.hpp"

#include <lava/magma/render-engine.hpp>

#include "../holders/swapchain-holder.hpp"
#include "../stages/present.hpp"
#include "../wrappers.hpp"

namespace lava::magma {
    class WindowRenderTarget::Impl final : public IRenderTarget::Impl {
    public:
        Impl(RenderEngine& engine, WsHandle handle, const Extent2d& extent);

        // IRenderTarget::Impl
        void init(uint32_t id) final;
        void update() final {}
        bool prepare() final;
        void render(vk::CommandBuffer commandBuffer) final;
        void draw(const std::vector<vk::CommandBuffer>& commandBuffers) const final;

        uint32_t addView(vk::ImageView imageView, vk::ImageLayout imageLayout, vk::Sampler sampler, const Viewport& viewport, uint32_t channelCount) final;
        void removeView(uint32_t viewId) final;
        void updateView(uint32_t viewId, vk::ImageView imageView, vk::ImageLayout imageLayout, vk::Sampler sampler) final;

        uint32_t id() const final { return m_id; }
        uint32_t currentBufferIndex() const final { return m_swapchainHolder.currentIndex(); }
        uint32_t buffersCount() const final { return m_swapchainHolder.imagesCount(); }

        // WindowRenderTarget
        inline Extent2d extent() const { return {m_windowExtent.width, m_windowExtent.height}; }
        void extent(const Extent2d& extent);
        WsHandle handle() const { return m_handle; }

    protected:
        // Internal
        void initFence();
        void initSurface();
        void initSwapchain();
        void initPresentStage();
        void recreateSwapchain();
        void createSemaphore();

    private:
        // References
        RenderEngine::Impl& m_engine;
        uint32_t m_id = -1u;
        WsHandle m_handle;

        // Resources
        Present m_presentStage;
        vk::UniqueSemaphore m_renderFinishedSemaphore;
        vk::UniqueFence m_fence;
        vk::UniqueSurfaceKHR m_surface;
        vulkan::SwapchainHolder m_swapchainHolder;
        vk::Extent2D m_windowExtent;
        bool m_shouldWaitForFences = true;
    };
}
