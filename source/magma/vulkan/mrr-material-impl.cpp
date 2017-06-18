#include "./mrr-material-impl.hpp"

#include <lava/chamber/logger.hpp>

#include "./image.hpp"
#include "./render-engine-impl.hpp"

#include <cstring>

namespace {
    void cleanAttribute(lava::MrrMaterial::Impl::Attribute& attribute)
    {
        if (attribute.type == lava::MrrMaterial::Impl::Attribute::Type::TEXTURE) {
            delete[] attribute.texture.pixels;
        }
        attribute.type = lava::MrrMaterial::Impl::Attribute::Type::NONE;
    }
}

using namespace lava;

MrrMaterial::Impl::Impl()
{
    m_baseColor.type = Attribute::Type::NONE;
}

MrrMaterial::Impl::~Impl()
{
    cleanAttribute(m_baseColor);

    // @todo Should be inside an attribute
    delete m_textureImageView;
    delete m_textureImageMemory;
    delete m_textureImage;
}

void MrrMaterial::Impl::init(RenderEngine& engine)
{
    m_engine = &engine.impl();

    auto& deviceCapsule = m_engine->device().capsule();
    m_textureImage = new vulkan::Capsule<VkImage>{deviceCapsule, vkDestroyImage};
    m_textureImageMemory = new vulkan::Capsule<VkDeviceMemory>{deviceCapsule, vkFreeMemory};
    m_textureImageView = new vulkan::Capsule<VkImageView>{deviceCapsule, vkDestroyImageView};
}

// @todo This should be a reference to a texture, so that it can be shared between materials
void MrrMaterial::Impl::baseColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels)
{
    cleanAttribute(m_baseColor);

    m_baseColor.type = Attribute::Type::TEXTURE;
    m_baseColor.texture.width = width;
    m_baseColor.texture.height = height;
    m_baseColor.texture.channels = channels;

    if (pixels.size() != width * height * channels) {
        logger.error("magma.vulkan.mrr-material") << "Image dimension for texture does not match provided data length."
                                                  << " Data: " << pixels.size() << " Dimensions: " << width << "x" << height
                                                  << " (" << static_cast<uint32_t>(channels) << ")" << std::endl;
    }

    m_baseColor.texture.pixels = new uint8_t[pixels.size()];
    memcpy(m_baseColor.texture.pixels, pixels.data(), pixels.size());

    rebindBaseColor();
}

void MrrMaterial::Impl::rebindBaseColor()
{
    if (m_baseColor.type != Attribute::Type::TEXTURE) return;

    if (m_baseColor.texture.channels != 4u) {
        logger.error("magma.culkan.mrr-material")
            << "Cannot handle texture with " << static_cast<uint32_t>(m_baseColor.texture.channels)
            << " channels. Only 4 is currently supported." << std::endl;
    }

    vulkan::Capsule<VkImage> stagingImage{m_engine->device().capsule(), vkDestroyImage};
    vulkan::Capsule<VkDeviceMemory> stagingImageMemory{m_engine->device().capsule(), vkFreeMemory};
    vulkan::Capsule<VkImageView> stagingImageView{m_engine->device().capsule(), vkDestroyImageView};

    vulkan::createImage(m_engine->device(), m_baseColor.texture.width, m_baseColor.texture.height, VK_FORMAT_R8G8B8A8_UNORM,
                        VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingImage,
                        stagingImageMemory);

    VkImageSubresource subresource = {};
    subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource.mipLevel = 0;
    subresource.arrayLayer = 0;

    VkSubresourceLayout stagingImageLayout;
    vkGetImageSubresourceLayout(m_engine->device(), stagingImage, &subresource, &stagingImageLayout);

    // Staging buffer
    vulkan::Capsule<VkBuffer> stagingBuffer{m_engine->device().capsule(), vkDestroyBuffer};
    vulkan::Capsule<VkDeviceMemory> stagingBufferMemory{m_engine->device().capsule(), vkFreeMemory};

    VkDeviceSize imageSize = m_baseColor.texture.width * m_baseColor.texture.height * 4;
    vulkan::createBuffer(m_engine->device(), imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                         stagingBufferMemory);

    void* data;
    vkMapMemory(m_engine->device(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, m_baseColor.texture.pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_engine->device(), stagingBufferMemory);

    // The real image
    vulkan::createImage(m_engine->device(), m_baseColor.texture.width, m_baseColor.texture.height, VK_FORMAT_R8G8B8A8_UNORM,
                        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *m_textureImage, *m_textureImageMemory);

    vulkan::transitionImageLayout(m_engine->device(), m_engine->commandPool(), *m_textureImage, VK_IMAGE_LAYOUT_PREINITIALIZED,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vulkan::copyBufferToImage(m_engine->device(), m_engine->commandPool(), stagingBuffer, *m_textureImage,
                              m_baseColor.texture.width, m_baseColor.texture.height);

    // Update image view
    vulkan::createImageView(m_engine->device(), *m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT,
                            *m_textureImageView);

    // Update descriptor set
    // @todo Have descriptor set per material type (e.g. 1 for MrrMaterial)
    // and find a way to bind the image by instance of material (during addCommands?)
    VkWriteDescriptorSet descriptorWrite = {};

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = *m_textureImageView;
    imageInfo.sampler = m_engine->textureSampler();

    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_engine->descriptorSet();
    descriptorWrite.dstBinding = 1;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = nullptr;
    descriptorWrite.pImageInfo = &imageInfo;
    descriptorWrite.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(m_engine->device(), 1, &descriptorWrite, 0, nullptr);
}

void MrrMaterial::Impl::addCommands(VkCommandBuffer /*commandBuffer*/)
{
    // @todo Bind whatever is needed!
}
