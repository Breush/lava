#pragma once

#include "./stages/environment-prefiltering-stage.hpp"

namespace lava::magma {
    class Texture;

    using TexturePtr = std::shared_ptr<Texture>;
}

namespace lava::magma {
    constexpr const uint32_t ENVIRONMENT_RADIANCE_SIZE = 512u;
    constexpr const uint32_t ENVIRONMENT_IRRADIANCE_SIZE = 64u;
    constexpr const uint32_t ENVIRONMENT_RADIANCE_MIP_LEVELS_COUNT = 9u; // Should be kept to log2(RADIANCE_SIZE).

    class Environment {
    public:
        Environment(Scene& scene, RenderEngine& engine);

        void init();
        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t descriptorSetIndex) const;
        void renderBasic(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t descriptorSetIndex) const;

        void set(const TexturePtr& texture);

    protected:
        void computeRadiance();
        void computeIrradiance();

        void createResources();

        void updateBindings();
        void updateBrdfLutBindings();
        void updateBasicBindings();

    private:
        // References
        Scene& m_scene;
        bool m_initialized = false;
        TexturePtr m_texture = nullptr;
        TexturePtr m_brdfLutTexture = nullptr;

        // Prefiltering
        EnvironmentPrefilteringStage m_radianceStage;
        EnvironmentPrefilteringStage m_irradianceStage;
        vulkan::ImageHolder m_radianceImageHolder;   // The specular part where roughness increases with mip levels.
        vulkan::ImageHolder m_irradianceImageHolder; // The diffuse part.

        // Descriptor
        vk::UniqueDescriptorSet m_basicDescriptorSet; // No mip levels, plain texture.
        vk::UniqueDescriptorSet m_descriptorSet;
    };
}
