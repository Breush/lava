#include "./image-holder.hpp"

#include "../helpers/command-buffer.hpp"
#include "../helpers/device.hpp"
#include "../render-engine-impl.hpp"
#include "../render-image-impl.hpp"

using namespace lava::magma::vulkan;
using namespace lava::magma;
using namespace lava::chamber;

ImageHolder::ImageHolder(const RenderEngine::Impl& engine)
    : m_engine(engine)
{
}

ImageHolder::ImageHolder(const RenderEngine::Impl& engine, const std::string& name)
    : ImageHolder(engine)
{
    m_name = name;
}

void ImageHolder::sampleCount(vk::SampleCountFlagBits sampleCount)
{
    if (m_sampleCount == sampleCount) return;
    m_sampleCount = sampleCount;
    m_sampleCountChanged = true;
}

void ImageHolder::create(ImageKind kind, vk::Format format, const vk::Extent2D& extent, uint8_t layersCount,
                         uint8_t mipLevelsCount)
{
    if (!m_sampleCountChanged &&
        m_kind == kind &&
        m_format == format &&
        m_extent == extent &&
        m_layersCount == layersCount &&
        m_mipLevelsCount == mipLevelsCount) {
        // Nothing to do that image was already created correctly
        return;
    }

    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

    m_kind = kind;
    m_format = format;
    m_extent = extent;
    m_layersCount = layersCount;
    m_mipLevelsCount = mipLevelsCount;
    m_sampleCountChanged = false;

    switch (format) {
    case vk::Format::eR8Unorm: {
        m_channels = 1u;
        m_channelBytesLength = 1u;
        break;
    }
    case vk::Format::eD16Unorm: {
        m_channels = 1u;
        m_channelBytesLength = 2u;
        break;
    }
    case vk::Format::eD32Sfloat: {
        m_channels = 1u;
        m_channelBytesLength = 4u;
        break;
    }
    // @fixme Should I use Srgb images instead? I have been using Unorm
    // with really no reason.
    case vk::Format::eR8G8B8A8Srgb:
    case vk::Format::eR8G8B8A8Unorm: {
        m_channels = 4u;
        m_channelBytesLength = 1u;
        break;
    }
    case vk::Format::eR16G16B16A16Sfloat: {
        m_channels = 4u;
        m_channelBytesLength = 2u;
        break;
    }
    case vk::Format::eR32G32B32A32Uint: {
        m_channels = 4u;
        m_channelBytesLength = 4u;
        break;
    }
    default: {
        logger.error("magma.vulkan.image-holder")
            << "Unknown format for image holder: " << vk::to_string(format) << "." << std::endl;
    }
    }

    m_imageBytesLength = m_extent.width * m_extent.height * m_channels * m_channelBytesLength;

    vk::ImageUsageFlags usageFlags;
    vk::MemoryPropertyFlags memoryPropertyFlags;
    vk::PipelineStageFlags srcStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
    vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
    vk::AccessFlags srcAccessMask;
    vk::AccessFlags dstAccessMask;

    // Depth
    if (kind == ImageKind::Depth) {
        m_aspect = vk::ImageAspectFlagBits::eDepth;
        usageFlags = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
        memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
        dstStageMask |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
        dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        m_layout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;
    }
    else {
        m_aspect = vk::ImageAspectFlagBits::eColor;
        memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        srcStageMask |= vk::PipelineStageFlagBits::eHost;
        dstStageMask |= vk::PipelineStageFlagBits::eTransfer;
        srcAccessMask = vk::AccessFlagBits::eHostWrite;
        dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        if (kind == ImageKind::Texture) {
            // @note Some data will be copied to the texture, so TransferDst should be written.
            usageFlags = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
            m_layout = vk::ImageLayout::eShaderReadOnlyOptimal;
        }
        else if (kind == ImageKind::TemporaryRenderTexture) {
            usageFlags = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc;
            m_layout = vk::ImageLayout::eUndefined;
        }
        else if (kind == ImageKind::RenderTexture) {
            usageFlags = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
            m_layout = vk::ImageLayout::eShaderReadOnlyOptimal;
        }
        else if (kind == ImageKind::Input) {
            usageFlags = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment;
            m_layout = vk::ImageLayout::eShaderReadOnlyOptimal;
        }
        else if (kind == ImageKind::Storage) {
            usageFlags = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled; // @fixme eSampled JUST TO ALLOW PRESENT
            m_layout = vk::ImageLayout::eGeneral;
        }
    }

    //----- Image

    vk::ImageCreateInfo imageCreateInfo;
    imageCreateInfo.imageType = vk::ImageType::e2D;
    imageCreateInfo.extent.width = m_extent.width;
    imageCreateInfo.extent.height = m_extent.height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = mipLevelsCount;
    imageCreateInfo.arrayLayers = layersCount;
    imageCreateInfo.format = format;
    imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
    imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageCreateInfo.usage = usageFlags;
    imageCreateInfo.samples = m_sampleCount;
    imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;

    if (layersCount == 6u) {
        imageCreateInfo.flags = vk::ImageCreateFlagBits::eCubeCompatible;
    }

    auto imageResult = m_engine.device().createImageUnique(imageCreateInfo);
    m_image = vulkan::checkMove(imageResult, "image-holder", "Unable to create image.");

    //---- Memory

    auto memRequirements = m_engine.device().getImageMemoryRequirements(m_image.get());

    vk::MemoryAllocateInfo allocInfo;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        findMemoryType(m_engine.physicalDevice(), memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

    auto memoryResult = m_engine.device().allocateMemoryUnique(allocInfo);
    m_memory = vulkan::checkMove(memoryResult, "image-holder", "Unable to allocate image memory.");

    // @fixme Do we /really/ want to bind all image memories?
    m_engine.deviceHolder().debugObjectName(m_memory.get(), "image-holder.memory." + m_name);
    m_engine.device().bindImageMemory(m_image.get(), m_memory.get(), 0);

    //----- Image view

    vk::ImageViewCreateInfo viewCreateInfo;
    viewCreateInfo.image = m_image.get();
    viewCreateInfo.viewType = (layersCount == 1) ? vk::ImageViewType::e2D : vk::ImageViewType::eCube;
    viewCreateInfo.format = format;
    viewCreateInfo.subresourceRange.aspectMask = m_aspect;
    viewCreateInfo.subresourceRange.baseMipLevel = 0;
    viewCreateInfo.subresourceRange.levelCount = mipLevelsCount;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount = layersCount;

    auto viewResult = m_engine.device().createImageViewUnique(viewCreateInfo);
    m_view = vulkan::checkMove(viewResult, "image-holder", "Unable to create image view.");

    //----- Transition

    if (m_layout != vk::ImageLayout::eUndefined) {
        auto commandBuffer = beginSingleTimeCommands(m_engine.device(), m_engine.commandPool());
        changeLayoutQuietly(m_layout, commandBuffer);
        endSingleTimeCommands(m_engine.device(), m_engine.graphicsQueue(), m_engine.commandPool(), commandBuffer);
    }

    if (!m_name.empty()) {
        m_engine.deviceHolder().debugObjectName(m_image.get(), m_name);
        m_engine.deviceHolder().debugObjectName(m_view.get(), m_name);
    }
}

void ImageHolder::copy(const void* data, uint8_t layersCount, uint8_t layerOffset)
{
    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

    if (layersCount == 0u) {
        layersCount = m_layersCount;
    }
    vk::DeviceSize size = m_imageBytesLength * layersCount;

    //----- Staging buffer

    BufferHolder stagingBufferHolder(m_engine, "image-holder.staging-buffer." + m_name);
    stagingBufferHolder.create(BufferKind::Staging, vulkan::BufferCpuIo::Direct, size);
    stagingBufferHolder.copy(data, size);

    //----- Copy indeed

    vk::BufferImageCopy bufferImageCopy;
    bufferImageCopy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    bufferImageCopy.imageSubresource.baseArrayLayer = layerOffset;
    bufferImageCopy.imageSubresource.layerCount = layersCount;
    bufferImageCopy.imageSubresource.mipLevel = 0;
    bufferImageCopy.imageExtent = vk::Extent3D{m_extent.width, m_extent.height, 1};

    auto commandBuffer = beginSingleTimeCommands(m_engine.device(), m_engine.commandPool());
    changeLayoutQuietly(vk::ImageLayout::eTransferDstOptimal, commandBuffer);
    commandBuffer.copyBufferToImage(stagingBufferHolder.buffer(), m_image.get(), vk::ImageLayout::eTransferDstOptimal, 1, &bufferImageCopy);
    changeLayoutQuietly(m_layout, commandBuffer);
    endSingleTimeCommands(m_engine.device(), m_engine.graphicsQueue(), m_engine.commandPool(), commandBuffer);
}

void ImageHolder::copy(vk::Image sourceImage, uint8_t layerOffset, uint8_t mipLevel)
{
    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

    vk::ImageCopy imageCopy;

    imageCopy.srcSubresource.aspectMask = m_aspect;
    imageCopy.srcSubresource.layerCount = 1;

    imageCopy.dstSubresource.aspectMask = m_aspect;
    imageCopy.dstSubresource.baseArrayLayer = layerOffset;
    imageCopy.dstSubresource.mipLevel = mipLevel;
    imageCopy.dstSubresource.layerCount = 1;

    auto width = m_extent.width;
    auto height = m_extent.height;
    for (auto i = 0u; i < mipLevel; ++i) {
        width /= 2;
        height /= 2;
    }

    imageCopy.extent.width = width;
    imageCopy.extent.height = height;
    imageCopy.extent.depth = 1;

    auto commandBuffer = beginSingleTimeCommands(m_engine.device(), m_engine.commandPool());
    changeLayoutQuietly(vk::ImageLayout::eTransferDstOptimal, commandBuffer);
    commandBuffer.copyImage(sourceImage, vk::ImageLayout::eTransferSrcOptimal, m_image.get(), vk::ImageLayout::eTransferDstOptimal, 1,
                            &imageCopy);
    changeLayoutQuietly(m_layout, commandBuffer);
    endSingleTimeCommands(m_engine.device(), m_engine.graphicsQueue(), m_engine.commandPool(), commandBuffer);
}

void ImageHolder::setup(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels,
                        uint8_t layersCount)
{
    setup(pixels.data(), width, height, channels, layersCount);
}

void ImageHolder::setup(const uint8_t* pixels, uint32_t width, uint32_t height, uint8_t channels, uint8_t layersCount)
{
    static std::mutex mutex;
    std::scoped_lock lock(mutex);

    vk::Format format = vk::Format::eUndefined;
    if (channels == 1u) {
        format = vk::Format::eR8Unorm;
    }
    else if (channels == 4u) {
        format = vk::Format::eR8G8B8A8Unorm;
    }
    else {
        logger.error("magma.vulkan.image-holder")
            << "Cannot handle image with " << static_cast<uint32_t>(channels) << " channels. "
            << "Currently supported: 1, 4." << std::endl;
    }

    if (layersCount != 1u && layersCount != 6u) {
        logger.error("magma.vulkan.image-holder")
            << "Cannot handle image with " << static_cast<uint32_t>(layersCount) << " layers. "
            << "Currently supported: 1, 6." << std::endl;
    }

    //----- Create

    vk::Extent2D extent = {width, height};
    create(vulkan::ImageKind::Texture, format, extent, layersCount);

    //----- Copy

    copy(pixels);
}

void ImageHolder::changeLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer)
{
    changeLayoutQuietly(imageLayout, commandBuffer);
    m_layout = imageLayout;
}

void ImageHolder::informLayout(vk::ImageLayout imageLayout)
{
    m_layout = imageLayout;
    m_lastKnownLayout = imageLayout;
}

RenderImage ImageHolder::renderImage(uint32_t uuid) const
{
    RenderImage renderImage;
    renderImage.impl().uuid(uuid);
    renderImage.impl().image(m_image.get());
    renderImage.impl().view(m_view.get());
    renderImage.impl().layout(m_layout);
    renderImage.impl().channelCount(m_channels);
    return renderImage;
}

void ImageHolder::savePng(const fs::Path& path, uint8_t layerOffset, uint8_t mipLevel)
{
    auto width = m_extent.width;
    auto height = m_extent.height;
    for (auto i = 0u; i < mipLevel; ++i) {
        width /= 2;
        height /= 2;
    }

    vk::DeviceSize size = width * height * m_channels * m_channelBytesLength;

    //----- Staging buffer

    BufferHolder stagingBufferHolder(m_engine, "image-holder.staging-buffer." + m_name);
    stagingBufferHolder.create(BufferKind::StagingTarget, vulkan::BufferCpuIo::Direct, size);

    //----- Copy from device

    vk::BufferImageCopy bufferImageCopy;
    bufferImageCopy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    bufferImageCopy.imageSubresource.layerCount = 1;
    bufferImageCopy.imageSubresource.baseArrayLayer = layerOffset;
    bufferImageCopy.imageSubresource.mipLevel = mipLevel;
    bufferImageCopy.imageExtent = vk::Extent3D{width, height, 1};

    auto commandBuffer = beginSingleTimeCommands(m_engine.device(), m_engine.commandPool());
    changeLayoutQuietly(vk::ImageLayout::eTransferSrcOptimal, commandBuffer);
    commandBuffer.copyImageToBuffer(m_image.get(), vk::ImageLayout::eTransferSrcOptimal, stagingBufferHolder.buffer(), 1, &bufferImageCopy);
    changeLayoutQuietly(m_layout, commandBuffer);
    endSingleTimeCommands(m_engine.device(), m_engine.graphicsQueue(), m_engine.commandPool(), commandBuffer);

    //----- Export as PNG

    std::vector<uint32_t> pixels(width * height);

    void* data = stagingBufferHolder.map(size);

    for (uint64_t j = 0u; j < height; ++j) {
        for (uint64_t i = 0u; i < width; ++i) {
            pixels[j * width + i] = extractPixelValue(data, width, i, j);
        }
    }

    stbi_write_png(path.string().c_str(), width, height, 4u, pixels.data(), 0u);

    stagingBufferHolder.unmap();
}

// ----- Internal

void ImageHolder::changeLayoutQuietly(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer)
{
    if (m_lastKnownLayout == imageLayout) return;

    // @noteDo not let the image layout going back to undefined!
    // As using ImageLayout::eUndefined as old layout loses the texels!
    if (imageLayout == vk::ImageLayout::eUndefined) return;

    // @note Not sure what this is for...
    vk::PipelineStageFlags stageMask = vk::PipelineStageFlagBits::eTopOfPipe;

    vk::ImageMemoryBarrier barrier;
    barrier.oldLayout = m_lastKnownLayout;
    barrier.newLayout = imageLayout;
    barrier.image = m_image.get();
    barrier.subresourceRange.levelCount = m_mipLevelsCount;
    barrier.subresourceRange.layerCount = m_layersCount;
    barrier.subresourceRange.aspectMask = m_aspect;

    commandBuffer.pipelineBarrier(stageMask, stageMask, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &barrier);

    m_lastKnownLayout = imageLayout;

    if (m_layout == vk::ImageLayout::eUndefined) {
        m_layout = imageLayout;
    }
}

// Normalized as RGBA
uint32_t ImageHolder::extractPixelValue(const void* data, uint64_t width, uint64_t i, uint64_t j) const
{
    uint32_t value = 0u;
    uint64_t offset = (j * width + i) * m_channels * m_channelBytesLength; // In bytes

    if (m_format == vk::Format::eR8G8B8A8Unorm ||
        m_format == vk::Format::eR8G8B8A8Srgb) {
        auto rgba = reinterpret_cast<const uint8_t*>(data) + offset;
        value = rgba[3];
        value = (value << 8u) + rgba[2];
        value = (value << 8u) + rgba[1];
        value = (value << 8u) + rgba[0];
    }
    else {
        logger.error("magma.vulkan.image-holder")
            << "Unsupported extracting pixel value with " << vk::to_string(m_format) << " image format. " << std::endl;
    }

    return value;
}
