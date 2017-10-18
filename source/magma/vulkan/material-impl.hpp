#pragma once

#include <lava/magma/material.hpp>

#include <lava/magma/render-scenes/render-scene.hpp>
#include <unordered_map>

#include "./holders/image-holder.hpp"
#include "./holders/ubo-holder.hpp"
#include "./ubos.hpp"

namespace lava::magma {
    /**
     * Vulkan-based implementation of magma::Material.
     */
    class Material::Impl {
    public:
        Impl(RenderScene& scene, const std::string& hrid);

        // Material
        void set(const std::string& uniformName, float value);
        void set(const std::string& uniformName, const glm::vec4& value);
        void set(const std::string& uniformName, const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height,
                 uint8_t channels);

        // Internal interface
        void init();
        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t descriptorSetIndex);

    protected:
        void updateBindings();

    private:
        struct Attribute {
            UniformType type = UniformType::UNKNOWN;
            uint32_t offset = -1u;
            bool enabled = false;
            UniformFallback fallback;
        };

    private:
        // References
        RenderScene::Impl& m_scene;
        bool m_initialized = false;

        // Descriptor
        vk::DescriptorSet m_descriptorSet;
        vulkan::UboHolder m_uboHolder;
        std::vector<std::unique_ptr<vulkan::ImageHolder>> m_imageHolders;

        // Data
        vulkan::MaterialUbo m_ubo;
        std::unordered_map<std::string, Attribute> m_attributes;
    };
}
