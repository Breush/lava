#pragma once

#include <lava/magma/render-engine.hpp>

#include "../wrappers.hpp"
#include "./buffer-holder.hpp"
#include "./descriptor-holder.hpp"

namespace lava::magma::vulkan {
    /**
     * Simple wrapper to be used as uniform buffer objects.
     */
    class UboHolder {
        struct UboSize {
            vk::DeviceSize size;
            uint32_t count;

            UboSize(vk::DeviceSize size)
                : UboSize(size, 1u)
            {
            }

            UboSize(vk::DeviceSize size, uint32_t count)
                : size(size)
                , count(count)
            {
            }
        };

    public:
        UboHolder() = delete;
        UboHolder(const RenderEngine::Impl& engine);

        /// Initialize the set to be used with all the sizes of the ubos.
        void init(vk::DescriptorSet descriptorSet, uint32_t bindingOffset, const std::vector<UboSize>& uboSizes);

        /// Copy data to the specified buffer.
        void copy(uint32_t bufferIndex, const void* data, vk::DeviceSize size, uint32_t arrayIndex = 0u);

        /// Helper function to copy data to the specified buffer.
        template <class T>
        void copy(uint32_t bufferIndex, const T& data, uint32_t arrayIndex = 0u);

    private:
        // References
        const RenderEngine::Impl& m_engine;
        vk::DescriptorSet m_descriptorSet = nullptr;
        uint32_t m_bindingOffset = 0u;

        // Resources
        std::vector<vulkan::BufferHolder> m_bufferHolders;

        // Internals
        vk::DeviceSize m_offsetAlignment;
    };
}

#include "./ubo-holder.inl"
