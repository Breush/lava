#pragma once

#include <lava/magma/cameras/vr-eye-camera.hpp>

#include "./i-camera-impl.hpp"

#include <lava/magma/render-scenes/render-scene.hpp>

#include "../../ubos.hpp"
#include "../../vr-engine.hpp"

namespace lava::magma {
    /**
     * Implementation of lava::VrEyeCamera.
     */
    class VrEyeCamera::Impl final : public ICamera::Impl {
    public:
        Impl(RenderScene& scene, Extent2d extent);
        ~Impl();

        // ICamera
        Extent2d extent() const { return {m_extent.width, m_extent.height}; }
        void extent(Extent2d extent);
        RenderImage renderImage() const;
        RenderImage depthRenderImage() const;
        PolygonMode polygonMode() const { return m_polygonMode; }
        void polygonMode(PolygonMode polygonMode);

        // ICamera::Impl
        bool vrAimed() const final { return true; }
        void init(uint32_t id) final;
        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t pushConstantOffset) const final;
        vk::Extent2D renderExtent() const final { return m_extent; }
        const glm::vec3& translation() const final { return m_translation; }
        const glm::mat4& viewTransform() const final { return m_viewTransform; }
        const glm::mat4& projectionTransform() const final { return m_projectionTransform; }
        const glm::mat4& inverseViewProjectionTransform() const final { return m_inverseViewProjectionTransform; }

        bool useFrustumCulling() const final { return false; } // @fixme Can't get that working
        const Frustum& frustum() const final { return m_frustum; }
        float nearClip() const final { return m_nearClip; };
        void nearClip(float nearClip) final { m_nearClip = nearClip; };
        float farClip() const final { return m_farClip; };
        void farClip(float farClip) final { m_farClip = farClip; };

        // Internal interface
        void update(VrEngine::Eye eye);
        void forceProjectionTransform(const glm::mat4& projectionTransform);
        void forceViewTransform(glm::mat4 viewTransform);
        void changeImageLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer);

    protected:
        void updateFrustum();
        void updateBindings();

    private:
        // References
        RenderScene::Impl& m_scene;
        uint32_t m_id = -1u;
        bool m_initialized = false;

        // Descriptor
        CameraUbo m_ubo;

        // Configuration
        vk::Extent2D m_extent;
        PolygonMode m_polygonMode = PolygonMode::Fill;

        // Attributes
        glm::vec3 m_translation = glm::vec3(0.f);
        glm::mat4 m_viewTransform = glm::mat4(1.f);
        glm::mat4 m_projectionTransform = glm::mat4(1.f);
        glm::mat4 m_inverseViewProjectionTransform = glm::mat4(1.f);
        Frustum m_frustum;
        float m_nearClip = 0.05f;
        float m_farClip = 100.f;
    };
}
