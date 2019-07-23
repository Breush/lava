#pragma once

#include <lava/magma/render-targets/vr-render-target.hpp>

#include "./i-render-target-impl.hpp"

#include <lava/magma/camera-controllers/vr-eye-camera-controller.hpp>
#include <lava/magma/render-engine.hpp>

#include "../holders/swapchain-holder.hpp"
#include "../wrappers.hpp"

namespace lava::magma {
    class VrRenderTarget::Impl final : public IRenderTarget::Impl {
    public:
        Impl(RenderEngine& engine);
        ~Impl();

        // IRenderTarget::Impl
        void init(uint32_t id) override final;
        bool prepare() override final;
        void render(vk::CommandBuffer commandBuffer) override final;
        void draw(const std::vector<vk::CommandBuffer>& commandBuffers) const override final;

        // @fixme Implement the views for this VrRenderTarget, somehow
        uint32_t addView(vk::ImageView, vk::ImageLayout, vk::Sampler, Viewport) final { return 0u; }
        void removeView(uint32_t) final {}
        void updateView(uint32_t, vk::ImageView, vk::ImageLayout, vk::Sampler) final {}

        uint32_t id() const final { return m_id; }
        uint32_t currentBufferIndex() const final { return 0; }
        uint32_t buffersCount() const final { return 1; }

        // VrRenderTarget
        void bindScene(Scene& scene);

    protected:
        // Internal
        void cleanup();
        void initFence();

    private:
        // References
        RenderEngine::Impl& m_engine;
        Scene* m_scene = nullptr;
        uint32_t m_id = -1u;

        // VR resources
        Camera* m_leftEyeCamera = nullptr;
        Camera* m_rightEyeCamera = nullptr;
        VrEyeCameraController m_leftEyeCameraController;
        VrEyeCameraController m_rightEyeCameraController;

        // Resources
        Extent2d m_extent;
        vulkan::Fence m_fence;
    };
}
