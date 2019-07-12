#pragma once

#include <lava/magma/cameras/orbit-camera.hpp>

#include "./i-camera-impl.hpp"

#include <lava/magma/render-scenes/render-scene.hpp>

#include "../../ubos.hpp"
#include "../holders/ubo-holder.hpp"

namespace lava::magma {
    /**
     * Implementation of lava::OrbitCamera.
     */
    class OrbitCamera::Impl final : public ICamera::Impl {
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
        const glm::mat4& viewTransform() const final { return m_viewTransform; }
        const glm::mat4& projectionTransform() const final { return m_projectionTransform; }
        const glm::mat4& inverseViewProjectionTransform() const final { return m_inverseViewProjectionTransform; }

        // ICamera::Impl
        bool vrAimed() const final { return false; }
        void init(uint32_t id) final;
        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t pushConstantOffset) const final;
        vk::Extent2D renderExtent() const final { return m_extent; }
        const glm::vec3& translation() const final { return m_translation; }
        bool useFrustumCulling() const final { return true; }
        const Frustum& frustum() const final { return m_frustum; }
        float nearClip() const final { return m_nearClip; };
        void nearClip(float nearClip) final { m_nearClip = nearClip; };
        float farClip() const final { return m_farClip; };
        void farClip(float farClip) final { m_farClip = farClip; };

        // OrbitCamera
        void translation(const glm::vec3& translation);
        void target(const glm::vec3& target);

    protected:
        void updateBindings();
        void updateViewTransform();
        void updateProjectionTransform();
        void updateFrustum();

    private:
        // References
        RenderScene::Impl& m_scene;
        uint32_t m_id = -1u;
        bool m_initialized = false;

        // Descriptor
        CameraUbo m_ubo;

        // Configuration
        vk::Extent2D m_extent;
        $attribute(glm::vec3, target);
        PolygonMode m_polygonMode = PolygonMode::Fill;
        float m_nearClip = 0.5f;
        float m_farClip = 100.f;

        // Attributes
        glm::vec3 m_translation = glm::vec3(0.f);
        glm::mat4 m_viewTransform = glm::mat4(1.f);
        glm::mat4 m_projectionTransform = glm::mat4(1.f);
        glm::mat4 m_inverseViewProjectionTransform = glm::mat4(1.f);
        Frustum m_frustum;
    };
}
