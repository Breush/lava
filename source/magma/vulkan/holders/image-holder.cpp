#include "./image-holder.hpp"

#include "../helpers/buffer.hpp"
#include "../helpers/command-buffer.hpp"
#include "../helpers/device.hpp"
#include "../render-engine-impl.hpp"
#include "../render-image-impl.hpp"

using namespace lava::magma::vulkan;
using namespace lava::magma;
using namespace lava::chamber;

ImageHolder::ImageHolder(const RenderEngine::Impl& engine)
    : m_engine(engine)
    , m_image{engine.device()}
    , m_memory{engine.device()}
    , m_view{engine.device()}
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

void ImageHolder::create(vk::Format format, vk::Extent2D extent, vk::ImageAspectFlagBits imageAspect, uint8_t layersCount,
                         uint8_t mipLevelsCount)
{
    if (!m_sampleCountChanged &&
        m_format == format &&
        m_extent == extent &&
        m_aspect == imageAspect &&
        m_layersCount == layersCount &&
        m_mipLevelsCount == mipLevelsCount) {
        // Nothing to do that image was already created correctly
        return;
    }

    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

    m_format = format;
    m_extent = extent;
    m_aspect = imageAspect;
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

    vk::ImageAspectFlags aspectFlags;
    vk::ImageUsageFlags usageFlags;
    vk::MemoryPropertyFlags memoryPropertyFlags;
    vk::PipelineStageFlags srcStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
    vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
    vk::AccessFlags srcAccessMask;
    vk::AccessFlags dstAccessMask;
    vk::ImageLayout oldLayout = vk::ImageLayout::eUndefined;

    // Depth
    if (imageAspect == vk::ImageAspectFlagBits::eDepth) {
        aspectFlags = vk::ImageAspectFlagBits::eDepth;
        usageFlags = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
        memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
        dstStageMask |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
        dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        m_layout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;
    }
    // Color
    else if (imageAspect == vk::ImageAspectFlagBits::eColor) {
        aspectFlags = vk::ImageAspectFlagBits::eColor;
        // @fixme eColorAttachment is not always necessary... we should be able to control that
        // And same goes for eSampled.
        usageFlags = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled
                     | vk::ImageUsageFlagBits::eTransferDst
                     | vk::ImageUsageFlagBits::eTransferSrc // @fixme We would love an option to not enable everything every time
                     | vk::ImageUsageFlagBits::eInputAttachment;
        memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        srcStageMask |= vk::PipelineStageFlagBits::eHost;
        dstStageMask |= vk::PipelineStageFlagBits::eTransfer;
        srcAccessMask = vk::AccessFlagBits::eHostWrite;
        dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        oldLayout = vk::ImageLayout::ePreinitialized;
        m_layout = vk::ImageLayout::eShaderReadOnlyOptimal; // Cannot use eColorAttachmentOptimal, somehow
    }
    else {
        logger.error("magma.vulkan.image-holder") << "Unknown image aspect for image holder. "
                                                  << "Valid ones are currently eDepth or eColor." << std::endl;
    }

    //----- Image

    vk::ImageCreateInfo imageInfo;
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent.width = m_extent.width;
    imageInfo.extent.height = m_extent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevelsCount;
    imageInfo.arrayLayers = layersCount;
    imageInfo.format = format;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.initialLayout = vk::ImageLayout::ePreinitialized;
    imageInfo.usage = usageFlags;
    imageInfo.samples = m_sampleCount;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;

    if (layersCount == 6u) {
        imageInfo.flags = vk::ImageCreateFlagBits::eCubeCompatible;
    }

    if (m_engine.device().createImage(&imageInfo, nullptr, m_image.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.image-holder") << "Failed to create image." << std::endl;
    }

    //---- Memory

    auto memRequirements = m_engine.device().getImageMemoryRequirements(m_image);

    vk::MemoryAllocateInfo allocInfo;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        findMemoryType(m_engine.physicalDevice(), memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

    if (m_engine.device().allocateMemory(&allocInfo, nullptr, m_memory.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.image-holder") << "Failed to allocate image memory." << std::endl;
    }

    // @fixme Do we /really/ want to bind all image memories?
    m_engine.device().bindImageMemory(m_image, m_memory, 0);

    //----- Image view

    vk::ImageViewCreateInfo viewInfo;
    viewInfo.image = m_image;
    viewInfo.viewType = (layersCount == 1) ? vk::ImageViewType::e2D : vk::ImageViewType::eCube;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevelsCount;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = layersCount;

    if (m_engine.device().createImageView(&viewInfo, nullptr, m_view.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.image") << "Failed to create image view." << std::endl;
    }

    //----- Transition

    vk::ImageMemoryBarrier barrier;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = m_layout;
    barrier.image = m_image;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.aspectMask = imageAspect;
    barrier.srcAccessMask = srcAccessMask;
    barrier.dstAccessMask = dstAccessMask;

    auto commandBuffer = beginSingleTimeCommands(m_engine.device(), m_engine.commandPool());
    commandBuffer.pipelineBarrier(srcStageMask, dstStageMask, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &barrier);
    endSingleTimeCommands(m_engine.device(), m_engine.graphicsQueue(), m_engine.commandPool(), commandBuffer);

    if (!m_name.empty()) {
        m_engine.deviceHolder().debugObjectName(m_image, m_name);
        m_engine.deviceHolder().debugObjectName(m_view, m_name);
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

    Buffer stagingBuffer(m_engine.device());
    DeviceMemory stagingBufferMemory(m_engine.device());

    vk::BufferUsageFlags usageFlags = vk::BufferUsageFlagBits::eTransferSrc;
    vk::MemoryPropertyFlags propertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

    createBuffer(m_engine.device(), m_engine.physicalDevice(), size, usageFlags, propertyFlags, stagingBuffer,
                 stagingBufferMemory);

    //----- Copy indeed

    void* targetData;
    vk::MemoryMapFlags memoryMapFlags;
    m_engine.device().mapMemory(stagingBufferMemory, 0, size, memoryMapFlags, &targetData);
    memcpy(targetData, data, size);
    m_engine.device().unmapMemory(stagingBufferMemory);

    vk::BufferImageCopy bufferImageCopy;
    bufferImageCopy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    bufferImageCopy.imageSubresource.baseArrayLayer = layerOffset;
    bufferImageCopy.imageSubresource.layerCount = layersCount;
    bufferImageCopy.imageExtent = vk::Extent3D{m_extent.width, m_extent.height, 1};

    auto commandBuffer = beginSingleTimeCommands(m_engine.device(), m_engine.commandPool());
    changeLayoutQuietly(vk::ImageLayout::eTransferDstOptimal, commandBuffer);
    commandBuffer.copyBufferToImage(stagingBuffer, m_image, vk::ImageLayout::eTransferDstOptimal, 1, &bufferImageCopy);
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
    commandBuffer.copyImage(sourceImage, vk::ImageLayout::eTransferSrcOptimal, m_image, vk::ImageLayout::eTransferDstOptimal, 1,
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
    create(format, extent, vk::ImageAspectFlagBits::eColor, layersCount);

    //----- Copy

    copy(pixels);
}

void ImageHolder::changeLayoutQuietly(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer) const
{
    // @note Not sure what this is for...
    vk::PipelineStageFlags stageMask = vk::PipelineStageFlagBits::eTopOfPipe;

    vk::ImageMemoryBarrier barrier;
    barrier.oldLayout = vk::ImageLayout::eUndefined; // @todo Well, that works, but it is definitly ugly
    barrier.newLayout = imageLayout;
    barrier.image = m_image;
    barrier.subresourceRange.levelCount = m_mipLevelsCount;
    barrier.subresourceRange.layerCount = m_layersCount;
    barrier.subresourceRange.aspectMask = m_aspect;

    commandBuffer.pipelineBarrier(stageMask, stageMask, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &barrier);
}

void ImageHolder::changeLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer)
{
    changeLayoutQuietly(imageLayout, commandBuffer);
    m_layout = imageLayout;
}

RenderImage ImageHolder::renderImage(uint32_t uuid) const
{
    RenderImage renderImage;
    renderImage.impl().uuid(uuid);
    renderImage.impl().image(m_image);
    renderImage.impl().view(m_view);
    renderImage.impl().layout(m_layout);
    renderImage.impl().channelCount(m_channels);
    return renderImage;
}

void ImageHolder::savePng(const fs::Path& path, uint8_t layerOffset, uint8_t mipLevel) const
{
    auto width = m_extent.width;
    auto height = m_extent.height;
    for (auto i = 0u; i < mipLevel; ++i) {
        width /= 2;
        height /= 2;
    }

    vk::DeviceSize size = width * height * m_channels * m_channelBytesLength;

    //----- Staging buffer

    Buffer stagingBuffer(m_engine.device());
    DeviceMemory stagingBufferMemory(m_engine.device());

    vk::BufferUsageFlags usageFlags = vk::BufferUsageFlagBits::eTransferDst;
    vk::MemoryPropertyFlags propertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

    createBuffer(m_engine.device(), m_engine.physicalDevice(), size, usageFlags, propertyFlags, stagingBuffer,
                 stagingBufferMemory);

    //----- Copy from device

    vk::BufferImageCopy bufferImageCopy;
    bufferImageCopy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    bufferImageCopy.imageSubresource.layerCount = 1;
    bufferImageCopy.imageSubresource.baseArrayLayer = layerOffset;
    bufferImageCopy.imageSubresource.mipLevel = mipLevel;
    bufferImageCopy.imageExtent = vk::Extent3D{width, height, 1};

    auto commandBuffer = beginSingleTimeCommands(m_engine.device(), m_engine.commandPool());
    changeLayoutQuietly(vk::ImageLayout::eTransferSrcOptimal, commandBuffer);
    commandBuffer.copyImageToBuffer(m_image, vk::ImageLayout::eTransferSrcOptimal, stagingBuffer, 1, &bufferImageCopy);
    changeLayoutQuietly(m_layout, commandBuffer);
    endSingleTimeCommands(m_engine.device(), m_engine.graphicsQueue(), m_engine.commandPool(), commandBuffer);

    //----- Export as PNG

    std::vector<uint32_t> pixels(width * height);

    void* data;
    vk::MemoryMapFlags memoryMapFlags;
    m_engine.device().mapMemory(stagingBufferMemory, 0, size, memoryMapFlags, &data);

    for (uint64_t j = 0u; j < height; ++j) {
        for (uint64_t i = 0u; i < width; ++i) {
            pixels[j * width + i] = extractPixelValue(data, width, i, j);
        }
    }

    stbi_write_png(path.c_str(), width, height, 4u, pixels.data(), 0u);

    m_engine.device().unmapMemory(stagingBufferMemory);
}

// ----- Internal

// Normalized as RGBA
uint32_t ImageHolder::extractPixelValue(const void* data, uint64_t width, uint64_t i, uint64_t j) const
{
    uint32_t value = 0u;
    uint64_t offset = (j * width + i) * m_channels * m_channelBytesLength; // In bytes

    if (m_format == vk::Format::eR8G8B8A8Unorm) {
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
