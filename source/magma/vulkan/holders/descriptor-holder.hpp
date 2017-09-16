#pragma once

#include <lava/magma/render-engine.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "../wrappers.hpp"

namespace lava::magma::vulkan {
    /**
     * Descriptor holder.
     */
    class DescriptorHolder final {
    public:
        DescriptorHolder() = delete;
        DescriptorHolder(const RenderEngine::Impl& engine);

        void init(uint32_t maxSetCount, vk::ShaderStageFlags shaderStageFlags);

        void storageBufferSizes(const std::vector<uint32_t>& storageBufferSizes) { m_storageBufferSizes = storageBufferSizes; }
        void uniformBufferSizes(const std::vector<uint32_t>& uniformBufferSizes) { m_uniformBufferSizes = uniformBufferSizes; }
        void combinedImageSamplerSizes(const std::vector<uint32_t>& combinedImageSamplerSizes)
        {
            m_combinedImageSamplerSizes = combinedImageSamplerSizes;
        }

        /// Allocate a single set from the associated pool.
        vk::DescriptorSet allocateSet() const;

        /// Update the specified storage buffer component.
        void updateSet(vk::DescriptorSet set, vk::Buffer buffer, vk::DeviceSize bufferSize, uint32_t storageBufferIndex);

        /// Update the specified combined image sampler component.
        void updateSet(vk::DescriptorSet set, vk::ImageView imageView, vk::Sampler sampler, vk::ImageLayout imageLayout,
                       uint32_t combinedImageSamplerIndex);

        /// Get the set layout.
        vk::DescriptorSetLayout setLayout() const { return m_setLayout; }

        /// Get the pool.
        vk::DescriptorPool pool() const { return m_pool; }

        /// Binding offsets.
        uint32_t storageBufferBindingOffset() const { return 0u; }
        uint32_t uniformBufferBindingOffset() const { return m_storageBufferSizes.size(); }
        uint32_t combinedImageSamplerBindingOffset() const { return m_storageBufferSizes.size() + m_uniformBufferSizes.size(); }

    protected:
        // References
        const RenderEngine::Impl& m_engine;

        // Configuration
        std::vector<uint32_t> m_storageBufferSizes;
        std::vector<uint32_t> m_uniformBufferSizes;
        std::vector<uint32_t> m_combinedImageSamplerSizes;

        // Resources
        vulkan::DescriptorSetLayout m_setLayout;
        vulkan::DescriptorPool m_pool;
    };
}
