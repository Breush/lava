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
        UboHolder() = default;
        UboHolder(const std::string& name) : m_name(name) {}
        UboHolder(const RenderEngine::Impl& engine) : m_engine(&engine) {}
        UboHolder(const RenderEngine::Impl& engine, const std::string& name) : m_engine(&engine), m_name(name) {}

        void engine(const RenderEngine::Impl& engine) { m_engine = &engine; }
        void name(const std::string& name) { m_name = name; }

        /// Initialize the set to be used with all the sizes of the ubos.
        void init(vk::DescriptorSet descriptorSet, uint32_t bindingOffset, const std::vector<UboSize>& uboSizes);

        /// Copy data to the specified buffer.
        void copy(uint32_t bufferIndex, const void* data, vk::DeviceSize size, uint32_t arrayIndex = 0u);

        /// Helper function to copy data to the specified buffer.
        template <class T>
        void copy(uint32_t bufferIndex, const T& data, uint32_t arrayIndex = 0u);

    private:
        // References
        const RenderEngine::Impl* m_engine = nullptr;
        std::string m_name;

        vk::DescriptorSet m_descriptorSet = nullptr;
        uint32_t m_bindingOffset = 0u;

        // Resources
        std::vector<vulkan::BufferHolder> m_bufferHolders;

        // Internals
        vk::DeviceSize m_offsetAlignment = 0u;
    };
}

#include "./ubo-holder.inl"
