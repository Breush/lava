#include "./rm-material-impl.hpp"

#include <lava/chamber/logger.hpp>
#include <vulkan/vulkan.h>

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

    void bindTextureDescriptorSet(vk::Device device, vk::DescriptorSet descriptorSet, uint32_t dstBinding, vk::Sampler sampler,
                                  vk::ImageView imageView)
    {
        vk::DescriptorImageInfo imageInfo;
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfo.imageView = imageView;
        imageInfo.sampler = sampler;

        vk::WriteDescriptorSet descriptorWrite;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = dstBinding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = nullptr;
        descriptorWrite.pImageInfo = &imageInfo;
        descriptorWrite.pTexelBufferView = nullptr;

        device.updateDescriptorSets(1u, &descriptorWrite, 0, nullptr);
    }
}

using namespace lava::magma;
using namespace lava::chamber;

RmMaterial::Impl::Impl(RenderEngine& engine)
    : m_engine(engine.impl())
    , m_uboHolder(m_engine)
    , m_normalImageHolder(m_engine)
    , m_albedoImageHolder(m_engine)
    , m_ormImageHolder(m_engine)
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
    m_descriptorSet = m_engine.materialDescriptorHolder().allocateSet();

    // Create uniform buffer
    m_uboHolder.init(m_descriptorSet, {sizeof(MaterialUbo)});
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
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, DESCRIPTOR_SET_INDEX, 1, &m_descriptorSet,
                                     0, nullptr);

    return nullptr;
}

//----- Private

void RmMaterial::Impl::updateBindings()
{
    // MaterialUbo
    MaterialUbo ubo;
    ubo.roughnessFactor = m_roughnessFactor;
    ubo.metallicFactor = m_metallicFactor;
    m_uboHolder.copy(0, ubo);

    // Samplers
    bindTextureDescriptorSet(m_engine.device(), m_descriptorSet, 1u, m_engine.dummySampler(),
                             (m_normal.type == Attribute::Type::TEXTURE) ? m_normalImageHolder.view()
                                                                         : m_engine.dummyNormalImageView());
    bindTextureDescriptorSet(m_engine.device(), m_descriptorSet, 2u, m_engine.dummySampler(),
                             (m_albedo.type == Attribute::Type::TEXTURE) ? m_albedoImageHolder.view()
                                                                         : m_engine.dummyImageView());
    bindTextureDescriptorSet(m_engine.device(), m_descriptorSet, 3u, m_engine.dummySampler(),
                             (m_orm.type == Attribute::Type::TEXTURE) ? m_ormImageHolder.view() : m_engine.dummyImageView());
}
