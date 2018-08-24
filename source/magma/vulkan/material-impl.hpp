#pragma once

#include <lava/magma/material.hpp>

#include <lava/magma/render-scenes/render-scene.hpp>
#include <lava/magma/texture.hpp>
#include <unordered_map>

#include "../uniform.hpp"
#include "./holders/image-holder.hpp"
#include "./holders/ubo-holder.hpp"
#include "./ubos.hpp"

namespace lava::magma {
    /**
     * Vulkan-based implementation of magma::Material.
     */
    class Material::Impl {
    private:
        struct Attribute {
            UniformType type = UniformType::Unknown;
            UniformFallback fallback;
            uint32_t offset = -1u;
            const Texture::Impl* texture = nullptr;
        };

    public:
        Impl(RenderScene& scene, const std::string& hrid);
        ~Impl();

        // Material
        void set(const std::string& uniformName, float value);
        void set(const std::string& uniformName, const glm::vec4& value);
        void set(const std::string& uniformName, const Texture& texture);

        // Internal interface
        void init();
        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t descriptorSetIndex);

    protected:
        Attribute& findAttribute(const std::string& uniformName);
        void updateBindings();

    private:
        // References
        RenderScene::Impl& m_scene;
        bool m_initialized = false;

        // Descriptor
        vk::DescriptorSet m_descriptorSet;
        vulkan::UboHolder m_uboHolder;

        // Data
        vulkan::MaterialUbo m_ubo;
        std::unordered_map<std::string, Attribute> m_attributes;
    };
}
