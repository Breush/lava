#pragma once

#include <lava/magma/render-engine.hpp>
#include <vulkan/vulkan.hpp>

#include "./wrappers.hpp"

namespace lava::magma::vulkan {
    /**
     * Descriptor holder.
     */
    class DescriptorHolder final {
    public:
        DescriptorHolder() = delete;
        DescriptorHolder(const RenderEngine::Impl& engine);

        void init(uint32_t uniformBufferCount, uint32_t combinedImageSamplerCount, uint32_t maxSetCount,
                  vk::ShaderStageFlags shaderStageFlags);

        /// Allocate a single set from the associated pool.
        vk::DescriptorSet allocateSet() const;

        /// Get the set layout.
        vk::DescriptorSetLayout setLayout() const { return m_setLayout; }

        /// Get the pool.
        vk::DescriptorPool pool() const { return m_pool; }

    protected:
        // References
        const RenderEngine::Impl& m_engine;

        // Resources
        vulkan::DescriptorSetLayout m_setLayout;
        vulkan::DescriptorPool m_pool;
    };
}
