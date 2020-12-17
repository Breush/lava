#pragma once

#include <lava/magma/ubos.hpp>

#include "../holders/image-holder.hpp"
#include "../holders/pipeline-holder.hpp"
#include "../holders/render-pass-holder.hpp"

namespace lava::magma {
    class Environment;
    class Scene;
    class RenderEngine;
}

namespace lava::magma {
    /**
     * Computes either mip-mapped radiance (specular) or irradiance (diffuse) from an environment cube-map.
     * This follows https://github.com/SaschaWillems/Vulkan-glTF-PBR behavior.
     */
    class EnvironmentPrefilteringStage final {
        constexpr static const uint32_t SAMPLES_COUNT = 32u;

    public:
        enum class Algorithm {
            Unknown,
            Radiance,
            Irradiance,
        };

    public:
        EnvironmentPrefilteringStage(Scene& scene, RenderEngine& engine);

        // Algorithm should be either "radiance" or "irrandiance".
        void init(Algorithm algorithm);
        void update(const vk::Extent2D& extent);
        void render(vk::CommandBuffer commandBuffer, uint8_t faceIndex);                   // For irradiance
        void render(vk::CommandBuffer commandBuffer, uint8_t faceIndex, uint8_t mipLevel); // For radiance

        vk::Image image() const { return m_imageHolder.image(); }
        vk::ImageView imageView() const { return m_imageHolder.view(); }
        vulkan::ImageHolder& imageHolder() { return m_imageHolder; }
        vk::RenderPass renderPass() const { return m_renderPassHolder.renderPass(); }

    protected:
        void initPass();

        void createResources();
        void createFramebuffers();

    private:
        // References
        Scene& m_scene;
        vk::Extent2D m_extent;
        Algorithm m_algorithm = Algorithm::Unknown;

        // Pass and subpasses
        vulkan::RenderPassHolder m_renderPassHolder;
        vulkan::PipelineHolder m_pipelineHolder;
        EnvironmentRadianceUbo m_radianceUbo;
        EnvironmentIrradianceUbo m_irradianceUbo;

        // Resources
        vulkan::ImageHolder m_imageHolder;
        vk::UniqueFramebuffer m_framebuffer;
    };
}
