#pragma once

#include "../../materials/fallback-material.hpp"

#include <lava/magma/render-scenes/render-scene.hpp>

#include "../holders/ubo-holder.hpp"
#include "./i-material-impl.hpp"

namespace lava::magma {
    /**
     * Vulkan-based implementation of lava::FallbackMaterial.
     */
    class FallbackMaterial::Impl : public IMaterial::Impl {
    public:
        Impl(RenderScene& scene);
        ~Impl();

        // IMaterial::Impl
        void init() override final;
        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                    uint32_t descriptorSetIndex) override final;

    protected:
        void updateBindings();

    private:
        // References
        RenderScene::Impl& m_scene;

        // Descriptor
        vk::DescriptorSet m_descriptorSet;
        vulkan::UboHolder m_uboHolder;
    };
}
