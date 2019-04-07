#pragma once

#include <lava/magma/render-engine.hpp>
#include <lava/magma/render-image.hpp>

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
        ImageHolder(const RenderEngine::Impl& engine, const std::string& name);

        /// Allocate all image memory for the specified format.
        void create(vk::Format format, vk::Extent2D extent, vk::ImageAspectFlagBits imageAspect);

        /// Copy data to the image.
        void copy(const void* data, vk::DeviceSize size);

        /// Allocate and copy from raw bytes.
        void setup(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels);
        void setup(const uint8_t* pixels, uint32_t width, uint32_t height, uint8_t channels);

        /// Adds to commands to the commandBuffer to change the layout. This won't change the return value of layout().
        void changeLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer);

        /// Generate a RenderImage from available information.
        RenderImage renderImage(uint32_t uuid) const;

    private:
        // References
        const RenderEngine::Impl& m_engine;

        // Resources
        vk::Extent2D m_extent;
        $attribute(vulkan::Image, image);
        $attribute(vulkan::DeviceMemory, memory);
        $attribute(vulkan::ImageView, view);
        $attribute(vk::ImageLayout, layout, = vk::ImageLayout::eUndefined);
        $attribute(vk::ImageAspectFlagBits, aspect);
    };
}
