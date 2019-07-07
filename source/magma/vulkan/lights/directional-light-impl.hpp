#pragma once

#include <lava/magma/lights/directional-light.hpp>

#include "./i-light-impl.hpp"

#include <lava/magma/render-scenes/render-scene.hpp>

#include "../holders/ubo-holder.hpp"

namespace lava::magma {
    /**
     * Implementation of magma::DirectionalLight.
     */
    class DirectionalLight::Impl : public ILight::Impl {
    public:
        Impl(RenderScene& scene);
        ~Impl();

        // ILight
        RenderImage shadowsRenderImage() const;

        // ILight::Impl
        void init(uint32_t id) final;
        void update() final;
        LightType type() const final { return LightType::Directional; };

        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t descriptorSetIndex) const final;

        // DirectionalLight
        void direction(const glm::vec3& direction);

    protected:
        void updateBindings();

    private:
        // References
        RenderScene::Impl& m_scene;
        bool m_initialized = false;
        uint32_t m_id = -1u;

        // Descriptor
        std::vector<vk::DescriptorSet> m_descriptorSets;
        std::vector<vulkan::UboHolder> m_uboHolders;
        bool m_uboDirty = false;
        uint32_t m_currentFrameId = 0u;

        // DirectionalLight
        $attribute(glm::vec3, direction);
    };
}
