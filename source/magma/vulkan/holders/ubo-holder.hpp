#pragma once

#include <lava/magma/render-engine.hpp>

#include "./buffer-holder.hpp"
#include "../wrappers.hpp"

namespace lava::magma::vulkan {
    /**
     * Simple wrapper to be used as uniform buffer objects.
     */
    class UboHolder {
    public:
        UboHolder() = delete;
        UboHolder(const RenderEngine::Impl& engine);

        /// Initialize the set to be used with all the sizes of the ubos.
        void init(vk::DescriptorSet descriptorSet, const std::vector<uint32_t>& uboSizes);

        /// Copy data to the specified buffer.
        void copy(uint32_t bufferIndex, const void* data, vk::DeviceSize size);

        /// Helper function to copy data to the specified buffer.
        template <class T>
        void copy(uint32_t bufferIndex, const T& data);

    private:
        // References
        const RenderEngine::Impl& m_engine;
        vk::DescriptorSet m_descriptorSet = nullptr;

        // Resources
        std::vector<vulkan::BufferHolder> m_bufferHolders;
    };
}

#include "./ubo-holder.inl"
