#pragma once

#include <lava/magma/lights/point-light.hpp>

#include "./i-light-impl.hpp"

#include <lava/core/macros.hpp>
#include <lava/magma/render-scenes/render-scene.hpp>

#include "../holders/ubo-holder.hpp"

namespace lava::magma {
    /**
     * Implementation of magma::PointLight.
     */
    class PointLight::Impl : public ILight::Impl {
    public:
        Impl(RenderScene& scene);
        ~Impl();

        // ILight
        RenderImage shadowsRenderImage() const;

        // ILight::Impl
        void init(uint32_t id) override final;
        LightType type() const override final { return LightType::Point; };

        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                    uint32_t descriptorSetIndex) const override final;

        // PointLight
        void translation(const glm::vec3& translation);
        void radius(const float& radius);

    protected:
        void updateBindings();

    private:
        // References
        RenderScene::Impl& m_scene;
        bool m_initialized = false;
        uint32_t m_id = -1u;

        // Descriptor
        vk::DescriptorSet m_descriptorSet;
        vulkan::UboHolder m_uboHolder;

        // ILight
        $attribute(glm::vec3, translation);
        $attribute(float, radius, = 1.f);
    };
}
