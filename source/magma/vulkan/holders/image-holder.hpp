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
        void create(vk::Format format, vk::Extent2D extent, vk::ImageAspectFlagBits imageAspect, uint8_t layersCount = 1u,
                    uint8_t mipLevelsCount = 1u);

        /**
         * Copy data to the image.
         * One can specify on which layer to start copying the data,
         * and how many to copy to.
         * Leaving layersCount to 0u means all layers.
         */
        void copy(const void* data, uint8_t layersCount = 0u, uint8_t layerOffset = 0u);

        /**
         * Copy from a source image.
         * The offset and level concerns the target image.
         * The source one is supposed to be not be mip mapped and have no layer offset.
         *
         * @note The sourceImage should be in ImageLayout::eTransferSrcOptimal.
         */
        void copy(vk::Image sourceImage, uint8_t layerOffset = 0u, uint8_t mipLevel = 0u);

        /// Allocate and copy from raw bytes.
        void setup(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels,
                   uint8_t layersCount = 1u);

        /**
         * Allocate and copy from raw bytes, expects each face one after another,
         * making pixels an array of size width * height * channels * layers.
         */
        void setup(const uint8_t* pixels, uint32_t width, uint32_t height, uint8_t channels, uint8_t layersCount = 1u);

        /// Adds to commands to the commandBuffer to change the layout. This won't change the return value of layout().
        void changeLayoutQuietly(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer) const;

        /// Adds to commands to the commandBuffer to change the layout. This WILL change the return value of layout().
        void changeLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer);

        /// Generate a RenderImage from available information.
        RenderImage renderImage(uint32_t uuid) const;

        /// Save the image as a PNG file.
        void savePng(const fs::Path& path, uint8_t layerOffset = 0u, uint8_t mipLevel = 0u) const;

    protected:
        uint32_t extractPixelValue(const void* data, uint64_t width, uint64_t i, uint64_t j) const;

    private:
        // References
        const RenderEngine::Impl& m_engine;
        std::string m_name;

        // Resources
        vk::Format m_format;
        vk::Extent2D m_extent;
        uint8_t m_layersCount = 1u;
        uint8_t m_mipLevelsCount = 1u;
        uint32_t m_imageBytesLength = 0u;
        uint8_t m_channels = 4u;
        uint8_t m_channelBytesLength = 1u;
        $attribute(vulkan::Image, image);
        $attribute(vulkan::DeviceMemory, memory);
        $attribute(vulkan::ImageView, view);
        $attribute(vk::ImageLayout, layout, = vk::ImageLayout::eUndefined);
        $attribute(vk::ImageAspectFlagBits, aspect);
        $property(vk::SampleCountFlagBits, sampleCount, = vk::SampleCountFlagBits::e1);
    };
}
