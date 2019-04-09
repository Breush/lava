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
    engine.deviceHolder().debugObjectName(m_image, name);
    engine.deviceHolder().debugObjectName(m_view, name);
}

void ImageHolder::create(vk::Format format, vk::Extent2D extent, vk::ImageAspectFlagBits imageAspect)
{
    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

    m_extent = extent;
    m_aspect = imageAspect;

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
        m_layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
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
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.initialLayout = vk::ImageLayout::ePreinitialized;
    imageInfo.usage = usageFlags;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;

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
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

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
}

void ImageHolder::copy(const void* data, vk::DeviceSize size)
{
    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

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
    bufferImageCopy.imageSubresource.layerCount = 1;
    bufferImageCopy.imageExtent = vk::Extent3D{m_extent.width, m_extent.height, 1};

    auto commandBuffer = beginSingleTimeCommands(m_engine.device(), m_engine.commandPool());
    changeLayout(vk::ImageLayout::eTransferDstOptimal, commandBuffer);
    commandBuffer.copyBufferToImage(stagingBuffer, m_image, vk::ImageLayout::eTransferDstOptimal, 1, &bufferImageCopy);
    changeLayout(m_layout, commandBuffer);
    endSingleTimeCommands(m_engine.device(), m_engine.graphicsQueue(), m_engine.commandPool(), commandBuffer);
}

void ImageHolder::setup(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels)
{
    setup(pixels.data(), width, height, channels);
}

void ImageHolder::setup(const uint8_t* pixels, uint32_t width, uint32_t height, uint8_t channels)
{
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

    //----- Create

    vk::Extent2D extent = {width, height};
    create(format, extent, vk::ImageAspectFlagBits::eColor);

    //----- Copy

    copy(pixels, width * height * channels);
}

void ImageHolder::changeLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer)
{
    PROFILE_FUNCTION();

    // @note Not sure what this is for...
    vk::PipelineStageFlags stageMask = vk::PipelineStageFlagBits::eTopOfPipe;

    vk::ImageMemoryBarrier barrier;
    barrier.oldLayout = vk::ImageLayout::eUndefined; // @todo Well, that works, but it is definitly ugly
    barrier.newLayout = imageLayout;
    barrier.image = m_image;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.aspectMask = m_aspect;

    commandBuffer.pipelineBarrier(stageMask, stageMask, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &barrier);
}

RenderImage ImageHolder::renderImage(uint32_t uuid) const
{
    RenderImage renderImage;
    renderImage.impl().uuid(uuid);
    renderImage.impl().image(m_image);
    renderImage.impl().view(m_view);
    renderImage.impl().layout(m_layout);
    return renderImage;
}
