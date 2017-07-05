#include "./rm-material-impl.hpp"

#include <lava/chamber/logger.hpp>
#include <vulkan/vulkan.h>

#include "../image.hpp"
#include "../render-engine-impl.hpp"

#include <cstring>

using namespace lava;

namespace {
    void cleanAttribute(magma::RmMaterial::Impl::Attribute& attribute)
    {
        if (attribute.type == magma::RmMaterial::Impl::Attribute::Type::TEXTURE) {
            delete[] attribute.texture.pixels;
        }
        attribute.type = magma::RmMaterial::Impl::Attribute::Type::NONE;
    }

    void setupTexture(magma::RmMaterial::Impl::Attribute::Texture& texture, const std::vector<uint8_t>& pixels, uint32_t width,
                      uint32_t height, uint8_t channels)
    {
        texture.width = width;
        texture.height = height;
        texture.channels = channels;

        if (pixels.size() != width * height * channels) {
            chamber::logger.error("magma.vulkan.rm-material")
                << "Image dimension for texture does not match provided data length."
                << " Data: " << pixels.size() << " Dimensions: " << width << "x" << height << " ("
                << static_cast<uint32_t>(channels) << ")" << std::endl;
        }

        texture.pixels = new uint8_t[pixels.size()];
        memcpy(texture.pixels, pixels.data(), pixels.size());
    }

    void setupTextureImage(magma::RmMaterial::Impl::Attribute::Texture& texture, magma::vulkan::Device& device,
                           VkCommandPool commandPool, magma::vulkan::Capsule<VkImage>& image,
                           magma::vulkan::Capsule<VkDeviceMemory>& imageMemory, magma::vulkan::Capsule<VkImageView>& imageView)
    {
        if (texture.channels != 4u) {
            chamber::logger.error("magma.vulkan.rm-material")
                << "Cannot handle texture with " << static_cast<uint32_t>(texture.channels)
                << " channels. Only 4 is currently supported." << std::endl;
        }

        magma::vulkan::Capsule<VkImage> stagingImage{device.capsule(), vkDestroyImage};
        magma::vulkan::Capsule<VkDeviceMemory> stagingImageMemory{device.capsule(), vkFreeMemory};
        magma::vulkan::Capsule<VkImageView> stagingImageView{device.capsule(), vkDestroyImageView};

        magma::vulkan::createImage(device, texture.width, texture.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR,
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
        magma::vulkan::Capsule<VkBuffer> stagingBuffer{device.capsule(), vkDestroyBuffer};
        magma::vulkan::Capsule<VkDeviceMemory> stagingBufferMemory{device.capsule(), vkFreeMemory};

        VkDeviceSize imageSize = texture.width * texture.height * 4;
        magma::vulkan::createBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                                    stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, texture.pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(device, stagingBufferMemory);

        // The real image
        magma::vulkan::createImage(device, texture.width, texture.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                   VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);

        magma::vulkan::transitionImageLayout(device, commandPool, image, VK_IMAGE_LAYOUT_PREINITIALIZED,
                                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        magma::vulkan::copyBufferToImage(device, commandPool, stagingBuffer, image, texture.width, texture.height);

        // Update image view
        magma::vulkan::createImageView(device, image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, imageView);
    }

    void bindTextureDescriptorSet(VkDescriptorSet& descriptorSet, uint32_t dstBinding, magma::vulkan::Device& device,
                                  magma::vulkan::Capsule<VkSampler>& sampler, magma::vulkan::Capsule<VkImageView>& imageView)
    {
        // Update descriptor set
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

        vkUpdateDescriptorSets(device, 1u, &descriptorWrite, 0, nullptr);
    }
}

using namespace lava::magma;
using namespace lava::chamber;

RmMaterial::Impl::Impl(RenderEngine& engine)
    : m_engine(engine.impl())
    , m_uniformStagingBuffer{m_engine.device().capsule(), vkDestroyBuffer}
    , m_uniformStagingBufferMemory{m_engine.device().capsule(), vkFreeMemory}
    , m_uniformBuffer{m_engine.device().capsule(), vkDestroyBuffer}
    , m_uniformBufferMemory{m_engine.device().capsule(), vkFreeMemory}
    , m_normalImage{m_engine.device().capsule(), vkDestroyImage}
    , m_normalImageMemory{m_engine.device().capsule(), vkFreeMemory}
    , m_normalImageView{m_engine.device().capsule(), vkDestroyImageView}
    , m_albedoImage{m_engine.device().capsule(), vkDestroyImage}
    , m_albedoImageMemory{m_engine.device().capsule(), vkFreeMemory}
    , m_albedoImageView{m_engine.device().capsule(), vkDestroyImageView}
    , m_ormImage{m_engine.device().capsule(), vkDestroyImage}
    , m_ormImageMemory{m_engine.device().capsule(), vkFreeMemory}
    , m_ormImageView{m_engine.device().capsule(), vkDestroyImageView}
{
    m_albedo.type = Attribute::Type::NONE;
    m_normal.type = Attribute::Type::NONE;
    m_orm.type = Attribute::Type::NONE;

    init();
    updateBindings();
}

RmMaterial::Impl::~Impl()
{
    cleanAttribute(m_albedo);
    cleanAttribute(m_normal);
    cleanAttribute(m_orm);
}

void RmMaterial::Impl::init()
{
    // Create descriptor set
    // @todo The linked shader should provide the layout
    VkDescriptorSetLayout layouts[] = {m_engine.materialDescriptorSetLayout()};
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_engine.materialDescriptorPool();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts;

    if (vkAllocateDescriptorSets(m_engine.device(), &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
        logger.error("magma.vulkan.rm-material") << "Failed to create descriptor set." << std::endl;
    }

    // Create uniform buffer
    VkDeviceSize bufferSize = sizeof(MaterialUbo);

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
    bufferInfo.range = sizeof(MaterialUbo);

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = 0u;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.pImageInfo = nullptr;
    descriptorWrite.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(m_engine.device(), 1u, &descriptorWrite, 0, nullptr);
}

void RmMaterial::Impl::roughness(float factor)
{
    m_roughnessFactor = factor;
    updateBindings();
}

void RmMaterial::Impl::metallic(float factor)
{
    m_metallicFactor = factor;
    updateBindings();
}

void RmMaterial::Impl::normal(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels)
{
    cleanAttribute(m_normal);

    m_normal.type = Attribute::Type::TEXTURE;
    setupTexture(m_normal.texture, pixels, width, height, channels);
    setupTextureImage(m_normal.texture, m_engine.device(), m_engine.commandPool(), m_normalImage, m_normalImageMemory,
                      m_normalImageView);
    updateBindings();
}

// @todo This should be a reference to a texture, so that it can be shared between materials
void RmMaterial::Impl::baseColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels)
{
    cleanAttribute(m_albedo);

    m_albedo.type = Attribute::Type::TEXTURE;
    setupTexture(m_albedo.texture, pixels, width, height, channels);
    setupTextureImage(m_albedo.texture, m_engine.device(), m_engine.commandPool(), m_albedoImage, m_albedoImageMemory,
                      m_albedoImageView);
    updateBindings();
}

void RmMaterial::Impl::metallicRoughnessColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height,
                                              uint8_t channels)
{
    cleanAttribute(m_orm);

    m_orm.type = Attribute::Type::TEXTURE;
    setupTexture(m_orm.texture, pixels, width, height, channels);
    setupTextureImage(m_orm.texture, m_engine.device(), m_engine.commandPool(), m_ormImage, m_ormImageMemory, m_ormImageView);
    updateBindings();
}

//----- IMaterial

IMaterial::UserData RmMaterial::Impl::render(IMaterial::UserData data)
{
    auto& commandBuffer = *reinterpret_cast<VkCommandBuffer*>(data);

    // __NOTE__: This presuppose that the correct shader is binded

    // Bind with the material descriptor set
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_engine.pipelineLayout(), DESCRIPTOR_SET_INDEX, 1,
                            &m_descriptorSet, 0, nullptr);

    return nullptr;
}

//----- Private

void RmMaterial::Impl::updateBindings()
{
    // MaterialUbo
    MaterialUbo materialUbo = {};
    materialUbo.roughnessFactor = m_roughnessFactor;
    materialUbo.metallicFactor = m_metallicFactor;

    void* data;
    vkMapMemory(m_engine.device(), m_uniformStagingBufferMemory, 0, sizeof(MaterialUbo), 0, &data);
    memcpy(data, &materialUbo, sizeof(MaterialUbo));
    vkUnmapMemory(m_engine.device(), m_uniformStagingBufferMemory);

    vulkan::copyBuffer(m_engine.device(), m_engine.commandPool(), m_uniformStagingBuffer, m_uniformBuffer, sizeof(MaterialUbo));

    // Samplers
    bindTextureDescriptorSet(m_descriptorSet, 1u, m_engine.device(), m_engine.textureSampler(),
                             (m_normal.type == Attribute::Type::TEXTURE) ? m_normalImageView : m_engine.dummyNormalImageView());
    bindTextureDescriptorSet(m_descriptorSet, 2u, m_engine.device(), m_engine.textureSampler(),
                             (m_albedo.type == Attribute::Type::TEXTURE) ? m_albedoImageView : m_engine.dummyImageView());
    bindTextureDescriptorSet(m_descriptorSet, 3u, m_engine.device(), m_engine.textureSampler(),
                             (m_orm.type == Attribute::Type::TEXTURE) ? m_ormImageView : m_engine.dummyImageView());
}
