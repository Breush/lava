#include "./rm-material-impl.hpp"

#include <lava/chamber/logger.hpp>
#include <vulkan/vulkan.h>

#include "../image.hpp"
#include "../render-engine-impl.hpp"
#include "../user-data-render.hpp"

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

    void bindTextureDescriptorSet(VkDescriptorSet& descriptorSet, uint32_t dstBinding, magma::vulkan::Device& device,
                                  magma::vulkan::Capsule<VkSampler>& sampler, const VkImageView& imageView)
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
    , m_uniformBufferHolder(m_engine.device(), m_engine.commandPool())
    , m_normalImageHolder(m_engine.device(), m_engine.commandPool())
    , m_albedoImageHolder(m_engine.device(), m_engine.commandPool())
    , m_ormImageHolder(m_engine.device(), m_engine.commandPool())
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
    m_uniformBufferHolder.create(vk::BufferUsageFlagBits::eUniformBuffer, sizeof(MaterialUbo));

    // Set it up
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = m_uniformBufferHolder.buffer().castOld(); // @cleanup HPP
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
    m_normalImageHolder.setup(pixels, width, height, channels);
    updateBindings();
}

// @todo This should be a reference to a texture, so that it can be shared between materials
void RmMaterial::Impl::baseColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels)
{
    cleanAttribute(m_albedo);

    m_albedo.type = Attribute::Type::TEXTURE;
    setupTexture(m_albedo.texture, pixels, width, height, channels);
    m_albedoImageHolder.setup(pixels, width, height, channels);
    updateBindings();
}

void RmMaterial::Impl::metallicRoughnessColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height,
                                              uint8_t channels)
{
    cleanAttribute(m_orm);

    m_orm.type = Attribute::Type::TEXTURE;
    setupTexture(m_orm.texture, pixels, width, height, channels);
    m_ormImageHolder.setup(pixels, width, height, channels);
    updateBindings();
}

//----- IMaterial

IMaterial::UserData RmMaterial::Impl::render(IMaterial::UserData data)
{
    auto& userData = *reinterpret_cast<UserDataRenderIn*>(data);
    const auto& commandBuffer = *userData.commandBuffer;
    const auto& pipelineLayout = *userData.pipelineLayout;

    // __NOTE__: This presuppose that the correct shader is binded

    // Bind with the material descriptor set
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, DESCRIPTOR_SET_INDEX, 1,
                            &m_descriptorSet, 0, nullptr);

    return nullptr;
}

//----- Private

void RmMaterial::Impl::updateBindings()
{
    // MaterialUbo
    MaterialUbo ubo = {};
    ubo.roughnessFactor = m_roughnessFactor;
    ubo.metallicFactor = m_metallicFactor;
    m_uniformBufferHolder.copy(ubo);

    // Samplers
    // @cleanup HPP
    bindTextureDescriptorSet(m_descriptorSet, 1u, m_engine.device(), m_engine.textureSampler(),
                             (m_normal.type == Attribute::Type::TEXTURE) ? m_normalImageHolder.view().castOld()
                                                                         : m_engine.dummyNormalImageView());
    bindTextureDescriptorSet(m_descriptorSet, 2u, m_engine.device(), m_engine.textureSampler(),
                             (m_albedo.type == Attribute::Type::TEXTURE) ? m_albedoImageHolder.view().castOld()
                                                                         : m_engine.dummyImageView());
    bindTextureDescriptorSet(m_descriptorSet, 3u, m_engine.device(), m_engine.textureSampler(),
                             (m_orm.type == Attribute::Type::TEXTURE) ? m_ormImageHolder.view().castOld()
                                                                      : m_engine.dummyImageView());
}
