#pragma once

#include <lava/magma/render-targets/vr-render-target.hpp>

#include "./i-render-target-impl.hpp"

#include <lava/magma/cameras/vr-eye-camera.hpp>
#include <lava/magma/render-engine.hpp>

#include "../holders/swapchain-holder.hpp"
#include "../render-scenes/render-scene-impl.hpp"
#include "../wrappers.hpp"

namespace lava::magma {
    class VrRenderTarget::Impl final : public IRenderTarget::Impl {
    public:
        Impl(RenderEngine& engine);

        // IRenderTarget::Impl
        void init(uint32_t id) override final;
        bool prepare() override final;
        void render(vk::CommandBuffer commandBuffer) override final;
        void draw(vk::CommandBuffer commandBuffer) const override final;

        // @fixme Implement the views for this VrRenderTarget, somehow
        uint32_t addView(vk::ImageView, vk::ImageLayout, vk::Sampler, Viewport) final { return 0u; }
        void removeView(uint32_t) final {}
        void updateView(uint32_t, vk::ImageView, vk::ImageLayout, vk::Sampler) final {}

        uint32_t id() const final { return m_id; }
        uint32_t currentBufferIndex() const final { return 0; }
        uint32_t buffersCount() const final { return 1; }

        // VrRenderTarget
        void bindScene(RenderScene& scene);

    protected:
        // Internal
        void initFence();

    protected:
        struct VrDevice {
            bool valid = false;           // Whether the device is valid.
            vr::ETrackedDeviceClass type; // Which device is being tracked.
            glm::mat4 transform;          // Last known absolute transform of the device.
        };

    private:
        // References
        RenderEngine::Impl& m_engine;
        RenderScene::Impl* m_scene = nullptr;
        uint32_t m_id = -1u;

        // VR resources
        std::vector<vr::TrackedDevicePose_t> m_vrDevicesPoses;
        std::vector<VrDevice> m_vrDevices;
        VrEyeCamera* m_leftEyeCamera = nullptr;
        VrEyeCamera* m_rightEyeCamera = nullptr;

        // Resources
        Extent2d m_extent;
        vulkan::Fence m_fence;
    };
}
