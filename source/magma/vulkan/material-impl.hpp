#pragma once

#include <lava/magma/material.hpp>

#include <lava/magma/render-scenes/render-scene.hpp>
#include <lava/magma/texture.hpp>

#include "../ubos.hpp"
#include "../uniform.hpp"
#include "./holders/image-holder.hpp"
#include "./holders/ubo-holder.hpp"

namespace lava::magma {
    /**
     * Vulkan-based implementation of magma::Material.
     */
    class Material::Impl {
    private:
        struct Attribute {
            UniformType type = UniformType::Unknown;
            UniformFallback fallback; // Value to use if user does not change it.
            UniformFallback value;    // Last known value.
            uint32_t offset = -1u;
            const Texture::Impl* texture = nullptr;
        };

    public:
        Impl(RenderScene& scene, const std::string& hrid);
        ~Impl();

        // Material
        void set(const std::string& uniformName, bool value);
        void set(const std::string& uniformName, uint32_t value);
        void set(const std::string& uniformName, float value);
        void set(const std::string& uniformName, const glm::vec2& value);
        void set(const std::string& uniformName, const glm::vec3& value);
        void set(const std::string& uniformName, const glm::vec4& value);
        void set(const std::string& uniformName, const Texture& texture);
        void set(const std::string& uniformName, const uint32_t* values, uint32_t size);

        const glm::vec4& get_vec4(const std::string& uniformName) const;

        // Internal interface
        void init();
        void update();
        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t descriptorSetIndex);

    protected:
        Attribute& findAttribute(const std::string& uniformName);
        const Attribute& findAttribute(const std::string& uniformName) const;
        void updateBindings();

    private:
        // References
        RenderScene::Impl& m_scene;
        bool m_initialized = false;

        // Descriptor
        std::vector<vk::DescriptorSet> m_descriptorSets;
        std::vector<vulkan::UboHolder> m_uboHolders;
        uint32_t m_currentFrameId = 0u;
        bool m_uboDirty = false;

        // Data
        MaterialUbo m_ubo;
        std::unordered_map<std::string, Attribute> m_attributes;
    };
}
