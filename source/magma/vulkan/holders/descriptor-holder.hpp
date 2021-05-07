#pragma once

#include <lava/magma/render-engine.hpp>

#include "../helpers/descriptor.hpp"
#include "../wrappers.hpp"

namespace lava::magma::vulkan {
    enum class DescriptorKind {
        Unknown,
        AccelerationStructure,
        StorageImage,
        StorageBuffer,
        UniformBuffer,
        CombinedImageSampler,
        InputAttachment,
    };

    /**
     * Descriptor holder.
     */
    class DescriptorHolder final {
    public:
        DescriptorHolder() = delete;
        DescriptorHolder(const RenderEngine::Impl& engine);

        void init(uint32_t maxSetCount, vk::ShaderStageFlags shaderStageFlags);

        void setSizes(DescriptorKind kind, const std::vector<uint32_t>& sizes);

        /// Allocate a single set from the associated pool.
        vk::UniqueDescriptorSet allocateSet(const std::string& debugName, bool dummyBinding = false) const;

        /// Update the specified acceleration structure component.
        void updateSet(vk::DescriptorSet set, vk::AccelerationStructureKHR accelerationStructure, uint32_t accelerationStructureIndex);

        /// Update the specified component (storage buffer or uniform buffer).
        void updateSet(vk::DescriptorSet set, DescriptorKind kind, vk::Buffer buffer, vk::DeviceSize bufferSize, uint32_t index, uint32_t arrayIndex = 0u);

        /// Update the specified combined image sampler component.
        void updateSet(vk::DescriptorSet set, vk::ImageView imageView, vk::Sampler sampler, vk::ImageLayout imageLayout,
                       uint32_t combinedImageSamplerIndex);

        /// Update the specified component (storage image or input attachment).
        void updateSet(vk::DescriptorSet set, DescriptorKind kind, vk::ImageView imageView, vk::ImageLayout imageLayout,
                       uint32_t index);

        /// Get the set layout.
        vk::DescriptorSetLayout setLayout() const { return m_setLayout.get(); }

        /// Get the pool.
        vk::DescriptorPool pool() const { return m_pool.get(); }

        /// Binding offsets.
        uint32_t offset(DescriptorKind kind) const { return m_bindings.at(kind).offset; }

    protected:
        struct Binding {
            uint32_t offset = 0u;
            uint32_t totalSize = 0u; // Will be computed during init()
            std::vector<uint32_t> sizes;
        };

    protected:
        // References
        const RenderEngine::Impl& m_engine;

        // Configuration
        std::unordered_map<DescriptorKind, Binding> m_bindings;
        uint32_t m_nextOffset = 0u;

        // Resources
        vk::UniqueDescriptorSetLayout m_setLayout;
        vk::UniqueDescriptorPool m_pool;
    };
}
