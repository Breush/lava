#pragma once

#include <lava/core/macros.hpp>
#include <lava/magma/render-engine.hpp>

#include "../wrappers.hpp"

namespace lava::magma::vulkan {
    /**
     * Simple wrapper around a vulkan ImageView,
     * holding its device memory and such.
     */
    class ImageHolder {
    public:
        ImageHolder() = delete;
        ImageHolder(const RenderEngine::Impl& engine);

        /// Allocate all image memory for the specified format.
        void create(vk::Format format, vk::Extent2D extent, vk::ImageAspectFlagBits imageAspect);

        /// Copy data to the image.
        void copy(const void* data, vk::DeviceSize size);

        /// Allocate and copy from raw bytes.
        void setup(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels);

    private:
        // References
        const RenderEngine::Impl& m_engine;

        // Resources
        vk::Extent2D m_extent;
        $attribute(vulkan::Image, image);
        $attribute(vulkan::DeviceMemory, memory);
        $attribute(vulkan::ImageView, view);
    };
}
