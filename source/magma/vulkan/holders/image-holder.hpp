#pragma once

#include <lava/magma/render-engine.hpp>
#include <lava/magma/render-image.hpp>

#include "../wrappers.hpp"

namespace lava::magma::vulkan {
    enum class ImageKind {
        Unknown,
        TemporaryRenderTexture,     // Color | Sampled | TransferSrc    (Undefined layout)
        RenderTexture,              // Color | Sampled                  (ShaderReadOnlyOptimal layout)
        Texture,                    // Color | Sampled | TransferDst    (ShaderReadOnlyOptimal layout)
        Input,                      // Color | Input                    (ShaderReadOnlyOptimal layout)
        Storage,                    // Storage                          (General layout)
        Depth,                      // DepthStencil | Sampled           (DepthStencilReadOnlyOptimal layout)
    };

    /**
     * Simple wrapper around a vulkan ImageView,
     * holding its device memory and such.
     */
    class ImageHolder {
    public:
        ImageHolder() = delete;
        ImageHolder(const RenderEngine::Impl& engine);
        ImageHolder(const RenderEngine::Impl& engine, const std::string& name);

        vk::Image image() const { return m_image.get(); }
        vk::ImageView view() const { return m_view.get(); }
        vk::SampleCountFlagBits sampleCount() const { return m_sampleCount; }
        void sampleCount(vk::SampleCountFlagBits m_sampleCount);

        /// Allocate all image memory for the specified format.
        void create(ImageKind kind, vk::Format format, const vk::Extent2D& extent, uint8_t layersCount = 1u,
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

        /// Adds to commands to the commandBuffer to change the layout. This WILL change the return value of layout().
        void changeLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer);

        /// Inform that the layout changed via another way.
        void informLayout(vk::ImageLayout imageLayout);

        /// Generate a RenderImage from available information.
        RenderImage renderImage(uint32_t uuid) const;

        /// Save the image as a PNG file.
        void savePng(const fs::Path& path, uint8_t layerOffset = 0u, uint8_t mipLevel = 0u);

    protected:
        // Adds to commands to the commandBuffer to change the layout. This won't change the return value of layout().
        void changeLayoutQuietly(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer);

        uint32_t extractPixelValue(const void* data, uint64_t width, uint64_t i, uint64_t j) const;

    private:
        // References
        const RenderEngine::Impl& m_engine;
        std::string m_name;

        // Resources
        ImageKind m_kind = ImageKind::Unknown;
        vk::Format m_format = vk::Format::eUndefined;
        vk::Extent2D m_extent;
        uint8_t m_layersCount = 1u;
        uint8_t m_mipLevelsCount = 1u;
        uint32_t m_imageBytesLength = 0u;
        uint8_t m_channels = 4u;
        uint8_t m_channelBytesLength = 1u;
        bool m_sampleCountChanged = true;
        vk::UniqueImage m_image;
        vk::UniqueDeviceMemory m_memory;
        vk::UniqueImageView m_view;
        $attribute(vk::ImageLayout, layout, = vk::ImageLayout::eUndefined);
        $attribute(vk::ImageAspectFlagBits, aspect, = vk::ImageAspectFlagBits::eMetadata);
        vk::ImageLayout m_lastKnownLayout = vk::ImageLayout::eUndefined;
        vk::SampleCountFlagBits m_sampleCount = vk::SampleCountFlagBits::e1;
    };
}
