#include "./mrr-material-impl.hpp"

#include <lava/chamber/logger.hpp>
#include <vulkan/vulkan.h>

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

    void setupTexture(lava::MrrMaterial::Impl::Attribute::Texture& texture, const std::vector<uint8_t>& pixels, uint32_t width,
                      uint32_t height, uint8_t channels)
    {
        texture.width = width;
        texture.height = height;
        texture.channels = channels;

        if (pixels.size() != width * height * channels) {
            lava::logger.error("magma.vulkan.mrr-material")
                << "Image dimension for texture does not match provided data length."
                << " Data: " << pixels.size() << " Dimensions: " << width << "x" << height << " ("
                << static_cast<uint32_t>(channels) << ")" << std::endl;
        }

        texture.pixels = new uint8_t[pixels.size()];
        memcpy(texture.pixels, pixels.data(), pixels.size());
    }

    void setupTextureImage(lava::MrrMaterial::Impl::Attribute::Texture& texture, lava::vulkan::Device& device,
                           VkCommandPool commandPool, lava::vulkan::Capsule<VkImage>& image,
                           lava::vulkan::Capsule<VkDeviceMemory>& imageMemory, lava::vulkan::Capsule<VkImageView>& imageView)
    {
        if (texture.channels != 4u) {
            lava::logger.error("magma.vulkan.mrr-material")
                << "Cannot handle texture with " << static_cast<uint32_t>(texture.channels)
                << " channels. Only 4 is currently supported." << std::endl;
        }

        lava::vulkan::Capsule<VkImage> stagingImage{device.capsule(), vkDestroyImage};
        lava::vulkan::Capsule<VkDeviceMemory> stagingImageMemory{device.capsule(), vkFreeMemory};
        lava::vulkan::Capsule<VkImageView> stagingImageView{device.capsule(), vkDestroyImageView};

        lava::vulkan::createImage(device, texture.width, texture.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR,
                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingImage,
                                  stagingImageMemory);

        VkImageSubresource subresource = {};
        subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresource.mipLevel = 0;
        subresource.arrayLayer = 0;

        VkSubresourceLayout stagingImageLayout;
        vkGetImageSubresourceLayout(device, stagingImage, &subresource, &stagingImageLayout);

        // Staging buffer
        lava::vulkan::Capsule<VkBuffer> stagingBuffer{device.capsule(), vkDestroyBuffer};
        lava::vulkan::Capsule<VkDeviceMemory> stagingBufferMemory{device.capsule(), vkFreeMemory};

        VkDeviceSize imageSize = texture.width * texture.height * 4;
        lava::vulkan::createBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                                   stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, texture.pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(device, stagingBufferMemory);

        // The real image
        lava::vulkan::createImage(device, texture.width, texture.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);

        lava::vulkan::transitionImageLayout(device, commandPool, image, VK_IMAGE_LAYOUT_PREINITIALIZED,
                                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        lava::vulkan::copyBufferToImage(device, commandPool, stagingBuffer, image, texture.width, texture.height);

        // Update image view
        lava::vulkan::createImageView(device, image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, imageView);
    }

    void bindTextureDescriptorSet(VkDescriptorSet& descriptorSet, uint32_t dstBinding, lava::vulkan::Device& device,
                                  lava::vulkan::Capsule<VkSampler>& sampler, lava::vulkan::Capsule<VkImageView>& imageView)
    {
        // Update descriptor set
        // @todo Have descriptor set per material type (e.g. 1 for MrrMaterial)
        // and find a way to bind the image by instance of material (during addCommands?)
        VkWriteDescriptorSet descriptorWrite = {};

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = imageView;
        imageInfo.sampler = sampler;

        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = dstBinding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = nullptr;
        descriptorWrite.pImageInfo = &imageInfo;
        descriptorWrite.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
}

using namespace lava;

MrrMaterial::Impl::Impl(RenderEngine& engine)
    : m_engine(engine.impl())
    , m_uniformStagingBuffer{m_engine.device().capsule(), vkDestroyBuffer}
    , m_uniformStagingBufferMemory{m_engine.device().capsule(), vkFreeMemory}
    , m_uniformBuffer{m_engine.device().capsule(), vkDestroyBuffer}
    , m_uniformBufferMemory{m_engine.device().capsule(), vkFreeMemory}
    , m_baseColorImage{m_engine.device().capsule(), vkDestroyImage}
    , m_baseColorImageMemory{m_engine.device().capsule(), vkFreeMemory}
    , m_baseColorImageView{m_engine.device().capsule(), vkDestroyImageView}
    , m_normalImage{m_engine.device().capsule(), vkDestroyImage}
    , m_normalImageMemory{m_engine.device().capsule(), vkFreeMemory}
    , m_normalImageView{m_engine.device().capsule(), vkDestroyImageView}
    , m_metallicRoughnessImage{m_engine.device().capsule(), vkDestroyImage}
    , m_metallicRoughnessImageMemory{m_engine.device().capsule(), vkFreeMemory}
    , m_metallicRoughnessImageView{m_engine.device().capsule(), vkDestroyImageView}
{
    m_baseColor.type = Attribute::Type::NONE;

    init();
}

MrrMaterial::Impl::~Impl()
{
    cleanAttribute(m_baseColor);
    cleanAttribute(m_normal);
    cleanAttribute(m_metallicRoughness);
}

void MrrMaterial::Impl::init()
{
    // Create uniform buffer
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    int bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    int memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vulkan::createBuffer(m_engine.device(), bufferSize, bufferUsageFlags, memoryPropertyFlags, m_uniformStagingBuffer,
                         m_uniformStagingBufferMemory);

    bufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    vulkan::createBuffer(m_engine.device(), bufferSize, bufferUsageFlags, memoryPropertyFlags, m_uniformBuffer,
                         m_uniformBufferMemory);

    // Set it up
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = m_uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_engine.descriptorSet();
    descriptorWrite.dstBinding = 1;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.pImageInfo = nullptr;
    descriptorWrite.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(m_engine.device(), 1u, &descriptorWrite, 0, nullptr);

    // Bind empty textures to materials @fixme Should be removed afterwards
    bindTextureDescriptorSet(m_engine.descriptorSet(), 2, m_engine.device(), m_engine.textureSampler(),
                             m_engine.dummyImageView());
    bindTextureDescriptorSet(m_engine.descriptorSet(), 3, m_engine.device(), m_engine.textureSampler(),
                             m_engine.dummyImageView());
    bindTextureDescriptorSet(m_engine.descriptorSet(), 4, m_engine.device(), m_engine.textureSampler(),
                             m_engine.dummyImageView());
}

void MrrMaterial::Impl::normal(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels)
{
    cleanAttribute(m_normal);

    m_normal.type = Attribute::Type::TEXTURE;
    setupTexture(m_normal.texture, pixels, width, height, channels);
    setupTextureImage(m_normal.texture, m_engine.device(), m_engine.commandPool(), m_normalImage, m_normalImageMemory,
                      m_normalImageView);
    bindTextureDescriptorSet(m_engine.descriptorSet(), 2, m_engine.device(), m_engine.textureSampler(), m_normalImageView);
}

// @todo This should be a reference to a texture, so that it can be shared between materials
void MrrMaterial::Impl::baseColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels)
{
    cleanAttribute(m_baseColor);

    m_baseColor.type = Attribute::Type::TEXTURE;
    setupTexture(m_baseColor.texture, pixels, width, height, channels);
    setupTextureImage(m_baseColor.texture, m_engine.device(), m_engine.commandPool(), m_baseColorImage, m_baseColorImageMemory,
                      m_baseColorImageView);
    bindTextureDescriptorSet(m_engine.descriptorSet(), 3, m_engine.device(), m_engine.textureSampler(), m_baseColorImageView);
}

void MrrMaterial::Impl::metallicRoughnessColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height,
                                               uint8_t channels)
{
    cleanAttribute(m_metallicRoughness);

    m_metallicRoughness.type = Attribute::Type::TEXTURE;
    setupTexture(m_metallicRoughness.texture, pixels, width, height, channels);
    setupTextureImage(m_metallicRoughness.texture, m_engine.device(), m_engine.commandPool(), m_metallicRoughnessImage,
                      m_metallicRoughnessImageMemory, m_metallicRoughnessImageView);
    bindTextureDescriptorSet(m_engine.descriptorSet(), 4, m_engine.device(), m_engine.textureSampler(),
                             m_metallicRoughnessImageView);
}

void MrrMaterial::Impl::addCommands(VkCommandBuffer /*commandBuffer*/)
{
    // @todo Bind whatever is needed!
}
